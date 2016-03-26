#include "common.h"
#include "x86/x86.h"
#include "x86/memory.h"

static PDE pde[NR_PDE] __attribute__((aligned(PAGE_SIZE)));
static PTE pte[NR_PTE] __attribute__((aligned(PAGE_SIZE)));

void test_kernel_page();

PDE *get_pdir()
{
	return pde;
}

PTE *get_ptab()
{
	return pte;
}

/* main target is to access all memory */
void init_page()
{
	int i;
	PDE *pdir = va_to_pa(pde);
	PTE *ptab = va_to_pa(pte);

	PTE *kptab = ptab;

	/* init the page directory firstly */
	for(i = 0; i < (PHY_SIZE / PD_MEM_SIZE); i++)
	{
		pdir[i].val = make_pde((uint32_t)kptab);
		pdir[i + (KERNEL_BASE / PD_MEM_SIZE)].val = make_pde((uint32_t)kptab);
		kptab += PD_PT_SIZE;
	}

	/* init the page table nextly(direct map to physical memory) */
	for(i = 0; i < PHY_SIZE; i += PAGE_SIZE)
	{
		ptab[i / PAGE_SIZE].val = make_pte(i);
	}

	CR3 cr3;
	cr3.val = read_cr3();
	cr3.page_directory_base = (((uint32_t)pdir) >> 12);
	write_cr3(cr3.val);

//	test_kernel_page();

	CR0 cr0;
	cr0.val = read_cr0();
	cr0.paging = 1;
	write_cr0(cr0.val);

	return;
}
/*
void test_kernel_page()
{
	CR0 cr0;
	cr0.val = read_cr0();
	assert(cr0.protect_enable);

	CR3 cr3;
	cr3.val = read_cr3();

	uint32_t dir_base = cr3.page_directory_base;
	assert((uint32_t)pdir == (dir_base << 12));

	int i;
	for(i = 0; i < PHY_SIZE; i += PAGE_SIZE)
	{
		uint32_t dir1 = (i >> 22) & 0x3ff;
		uint32_t dir_addr = (dir_base << 20) + dir1 * 4;

		PDE *kpde = (void *)dir_addr;

	}
}
*/
