#include "common.h"
#include "x86/x86.h"
#include "x86/memory.h"
#include "process/process.h"

#define NR_PUDIR 1024

PTE *get_ptab();
PDE *get_pdir();
PTE *apply_phypage();

struct PHYPAGE {
	PTE *ptab;
	struct PHYPAGE *prev, *next;
};

static struct PHYPAGE *phypage_free, *phypage_full, phypage[PHY_SIZE / PD_MEM_SIZE];

static bool udir_dirty[NR_PROGRESS];
static PDE pudir[NR_PROGRESS][NR_PUDIR] __attribute((aligned(PAGE_SIZE)));

HANDLE apply_udir()
{
	int i;
	for(i = 0; i < NR_PROGRESS; i++)
	{
		if(!udir_dirty[i])
		{
			udir_dirty[i] = true;
			return i;
		}
	}
	return INVALID_HANDLE_VALUE;
}

/* allocate virtual page for process */
HANDLE mm_alloc(HANDLE hProgress, uint32_t vaddr, uint32_t size)
{
	int i;
	if(hProgress == INVALID_HANDLE_VALUE)
		hProgress = apply_udir();
	assert(hProgress < NR_PROGRESS);

	PDE *kpudir = pudir[hProgress];

	uint32_t ex_bytes = vaddr % PD_MEM_SIZE;
	vaddr -= ex_bytes;
	size += ex_bytes;

	//bug here
	for(i = vaddr; i < vaddr + size; i += PD_MEM_SIZE)
	{
		if(!kpudir[i / PD_MEM_SIZE].present)
		{
			PTE *tmp = apply_phypage();
			kpudir[i / PD_MEM_SIZE].val = make_usr_pde((uint32_t)va_to_pa(tmp));
		}
	}
	return hProgress;
}

void load_udir(HANDLE hProgress)
{
	assert(hProgress < NR_PROGRESS);
	CR3 cr3;
	cr3.val = read_cr3();
	cr3.page_directory_base = (((uint32_t)va_to_pa(pudir[hProgress])) >> 12);
	write_cr3(cr3.val);
}

void init_udir()
{
	//kernel space is shared by all process
	int i, j;
	PDE * kpdir = get_pdir();
	for(i = 0; i < NR_PROGRESS; i++)
	{
		for(j = 0; j < NR_PUDIR; j++)
		{
			pudir[i][j].val = kpdir[j].val;
		}
	}

	for(i = 0; i < NR_PROGRESS; i++)
	{
		udir_dirty[i] = false;
	}
}

void init_mm()
{
	int i;
	PTE *kptab = get_ptab();
	uint32_t phypage_base = (KERNEL_PHYBASE + KERNEL_SIZE) / PD_MEM_SIZE + 1;
	for(i = phypage_base; i < PHY_SIZE / PD_MEM_SIZE - 1; i++)
	{
		phypage[i].ptab = &kptab[i * PD_PT_SIZE];
		phypage[i].next = &phypage[i + 1];
		phypage[i + 1].prev = &phypage[i];
	}

	phypage[i].ptab = &kptab[i * PD_PT_SIZE];
	phypage[i].next = NULL;
	
	phypage[phypage_base].prev = NULL;

	phypage_free = &phypage[phypage_base];
	phypage_full = NULL;
}

PTE *apply_phypage()
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

	return tmp->ptab;
}

void free_phypage(uint32_t phyaddr)
{
	assert(phyaddr >= (KERNEL_PHYBASE + KERNEL_SIZE) && phyaddr < PHY_SIZE);
	struct PHYPAGE * target = &phypage[phyaddr / PAGE_SIZE];

	struct PHYPAGE * tmp = target;
	struct PHYPAGE * next = target->next;
	struct PHYPAGE * prev = target->prev;

	if(prev != NULL)
		target->prev->next = target->next;
	if(next != NULL)
		target->next->prev = prev;

	target->prev = NULL;
	target->next = phypage_free;
	if(phypage_free != NULL)
		phypage_free->prev = target;
	phypage_free = target;
	if(tmp == phypage_full)
		phypage_full = next;
}

