#include "common.h"
#include "x86/x86.h"
#include "x86/memory.h"
#include "device/video.h"

PDE *get_pdir();
PTE vmmap[SCR_BITSIZE / PAGE_SIZE + 1] __attribute__((aligned(PAGE_SIZE)));

void create_vmmap()
{
	/* create map from virtual memory [VMEM, VMEM + SCR_BITSIZE) to physical memory [VMEM, VMEM + SCR_BITSIZE) */

	int i;
	PDE *kpdir = get_pdir();
	PTE *kptab = va_to_pa(vmmap);
	for(i = ((VMEM >> 22) & 0x3ff); i <= (((VMEM + SCR_BITSIZE) >> 22) & 0x3ff); i++)
	{
		kpdir[i].val = make_pde((uint32_t)kptab);
		kptab += PD_PT_SIZE;
	}

	for(i = 0; i <= SCR_BITSIZE; i += PAGE_SIZE)
	{
		vmmap[i / PAGE_SIZE].val = make_pte(VMEM + i);
	}
}
