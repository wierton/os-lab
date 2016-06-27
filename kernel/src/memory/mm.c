#include "common.h"
#include "math.h"
#include "x86/x86.h"
#include "x86/memory.h"
#include "proc/proc.h"

#define NRT 1024
#define NR_PUTAB 64

PTE *get_ptab();
PDE *get_pdir();
uint32_t apply_phypage(HANDLE hOwner);

struct PHYPAGE {
	uint32_t paddr, hOwner;
	struct PHYPAGE *prev, *next;
};

static struct PHYPAGE *phypage_free, *phypage_full, phypage[PHY_SIZE / PT_MEM_SIZE];

static bool udir_dirty[NR_PROCESS];
static bool utab_dirty[NR_PUTAB];

static PDE pudir[NR_PROCESS][NRT] __attribute((aligned(PAGE_SIZE)));
static PTE putab[NR_PUTAB][NRT] __attribute((aligned(PAGE_SIZE)));

HANDLE apply_udir()
{
	int i;
	for(i = 0; i < NR_PROCESS; i++)
	{
		if(!udir_dirty[i])
		{
			udir_dirty[i] = true;
			return i;
		}
	}
	assert(0);
	return INVALID_HANDLE_VALUE;
}

PDE *get_udir(HANDLE hProcess)
{
	assert(hProcess < NR_PROCESS);
	return pudir[hProcess];
}

PTE *apply_utab()
{
	int i;
	for(i = 0; i < NR_PUTAB; i++)
	{
		if(!utab_dirty[i])
		{
			utab_dirty[i] = true;
			return putab[i];
		}
	}
	assert(0);
	return NULL;
}

/* allocate virtual page for process */
HANDLE mm_alloc(HANDLE hProcess, uint32_t vaddr, uint32_t size)
{
	int i, j;
	assert(hProcess < NR_PROCESS);

	PDE *kpudir = pudir[hProcess];

	uint32_t ex_dir_bytes = vaddr % PD_MEM_SIZE;
	uint32_t dir_op = vaddr - ex_dir_bytes;
	uint32_t dir_size = size + ex_dir_bytes;

	uint32_t ex_bytes = vaddr % PT_MEM_SIZE;
	vaddr -= ex_bytes;
	size += ex_bytes;

	for(i = dir_op; i < dir_op + dir_size; i += PD_MEM_SIZE)
	{
		PTE *tmp_tab = (void *)(kpudir[i / PD_MEM_SIZE].page_frame << 12);
		if(!kpudir[i / PD_MEM_SIZE].present)
		{
			tmp_tab = apply_utab();
			kpudir[i / PD_MEM_SIZE].val = make_usr_pde((uint32_t)va_to_pa(tmp_tab));
		}

		uint32_t j_op = max(i, vaddr);
		uint32_t j_ed = min(i + PD_MEM_SIZE, vaddr + size);

		for(j = j_op; j < j_ed; j += PT_MEM_SIZE)
		{
			uint32_t paddr = apply_phypage(hProcess);
			tmp_tab[(j - i) / PT_MEM_SIZE].val = make_usr_pte(paddr);
		}
	}
	return hProcess;
}

PDE *load_udir(HANDLE hProgress)
{
	assert(hProgress < NR_PROCESS);
	CR3 cr3;
	PDE *old_pdir;
	cr3.val = read_cr3();
	old_pdir = (void *)(pa_to_va(cr3.page_directory_base << 12));
	cr3.page_directory_base = (((uint32_t)va_to_pa(pudir[hProgress])) >> 12);
	write_cr3(cr3.val);
	return old_pdir;
}

void init_udir()
{
	/* kernel space is shared by all process */
	int i, j;
	PDE * kpdir = get_pdir();
	for(i = 0; i < NR_PROCESS; i++)
	{
		for(j = 0; j < NRT; j++)
		{
			pudir[i][j].val = kpdir[j].val;
		}
	}

	for(i = 0; i < NR_PROCESS; i++)
	{
		udir_dirty[i] = false;
	}
}

void init_utab()
{
	int i;

	for(i = 0; i < NR_PUTAB; i++)
	{
		utab_dirty[i] = false;
	}
}

void init_phypage()
{
	int i;
	uint32_t phypage_base = (KERNEL_PHYBASE + KERNEL_SIZE) / PT_MEM_SIZE + 1;
	for(i = phypage_base; i < PHY_SIZE / PT_MEM_SIZE - 1; i++)
	{
		phypage[i].hOwner = INVALID_OWNER;
		phypage[i].paddr = i * PT_MEM_SIZE;
		phypage[i].next = &phypage[i + 1];
		phypage[i + 1].prev = &phypage[i];
	}

	phypage[i].hOwner = INVALID_OWNER;
	phypage[i].paddr = i * PT_MEM_SIZE;
	phypage[i].next = NULL;
	
	phypage[phypage_base].prev = NULL;

	phypage_free = &phypage[phypage_base];
	phypage_full = NULL;
}

void init_mm()
{
	init_udir();
	init_utab();
	init_phypage();
}

uint32_t apply_phypage(HANDLE hOwner)
{
	struct PHYPAGE * tmp = phypage_free;
	assert(phypage_free != NULL);
	phypage_free = phypage_free->next;
	if(phypage_free != NULL)
		phypage_free->prev = NULL;
	tmp->next = phypage_full;
	if(phypage_full != NULL)
		phypage_full->prev = tmp;
	phypage_full = tmp;

	tmp->hOwner = hOwner;
	return tmp->paddr;
}

void free_phypage(uint32_t phyaddr)
{
	assert(phyaddr >= (KERNEL_PHYBASE + KERNEL_SIZE) && phyaddr < PHY_SIZE);
	struct PHYPAGE * target = &phypage[phyaddr / PAGE_SIZE];

	struct PHYPAGE * tmp = target;
	struct PHYPAGE * next = target->next;
	struct PHYPAGE * prev = target->prev;

	tmp->hOwner = INVALID_OWNER;

	if(prev != NULL)
		prev->next = next;
	if(next != NULL)
		next->prev = prev;

	target->prev = NULL;
	target->next = phypage_free;
	if(phypage_free != NULL)
		phypage_free->prev = target;
	phypage_free = target;
	if(tmp == phypage_full)
		phypage_full = next;
}

void free_memspace(HANDLE hProcess)
{
	assert(hProcess < NR_PROCESS);

	int i, j, k;
	/* free physical page */
	struct PHYPAGE *tmp = phypage_full;
	udir_dirty[hProcess] = false;
	while(tmp != NULL)
	{
		struct PHYPAGE * wait_free = tmp;
		tmp = tmp->next;
		if(wait_free->hOwner == hProcess)
		{
			free_phypage(wait_free->paddr);
		}
	}

	PHINFO ph = get_memh(hProcess);
	MEMH *pm = ph.pm;
	uint32_t pm_num = ph.pm_num;

	/* free user tab and user dir*/
	for(i = 0; i < pm_num; i++)
	{
		uint32_t vaddr_ed = pm[i].p_vaddr + pm[i].p_memsz;
		uint32_t vaddr_op = pm[i].p_vaddr - pm[i].p_vaddr % PAGE_SIZE;
		for(j = vaddr_op; j < vaddr_ed; j += PD_MEM_SIZE)
		{
			uint32_t nr_dir = j / PD_MEM_SIZE;
			
			udir_dirty[nr_dir] = false;
			pudir[hProcess][nr_dir].present = false;

			PTE *pte = (void *)(pudir[hProcess][nr_dir].page_frame << 12);
			utab_dirty[(uint32_t)(pte - putab[0]) / (NRT * sizeof(PTE))] = false;
			uint32_t min_jk = min(j + PD_MEM_SIZE, vaddr_ed);
			for(k = j; k < min_jk; k += PAGE_SIZE)
			{
				uint32_t nr_tab = (k & 0x3fffff) / PAGE_SIZE;
				pte[nr_tab].present = false;
			}
		}
	}	
}

uint32_t page_translate(HANDLE hProc, uint32_t vaddr)
/* translate given addr from virtual addr to physical addr */
{
	/* for given hProc, pagedir baseaddr is pudir[hProc],
	 * we can get page table base addr by pudir[hProc][vaddr:pdir]
	 * */
	assert(hProc < NR_PROCESS);
	PDE pdir = pudir[hProc][(vaddr >> 22) & 0x3ff];
	PTE ptab;

	uint32_t phyoff = vaddr & 0xfff;
	uint32_t pagetab = (vaddr >> 12) & 0x3ff;
	
	if(!pdir.present)
	{
		return 0xffffffff;
	}

	/* read page table */
	ptab.val = ((uint32_t *)((uint32_t)pa_to_va(pdir.page_frame << 12) | pagetab * 4))[0];
	if(!ptab.present)
	{
		return 0xffffffff;
	}

	/* calc physic address */
	return (ptab.page_frame << 12) | phyoff;
}

void copy_phypage(HANDLE hSrc, HANDLE hDst, uint32_t vaddr)
{
	/* operation below assume that base addr 
	 * in segmention transformation is zero */
	uint32_t src = (page_translate(hSrc, vaddr) & (0xfffff000));
	uint32_t dst = (page_translate(hDst, vaddr) & (0xfffff000));

	int i;
	for(i = 0; i < PAGE_SIZE; i++)
	{
	//	printk("[%d, %d] ", dst, src);
		((uint32_t *)(dst + i))[0] = ((uint32_t *)(src + i))[0];
	}
}
