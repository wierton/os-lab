#include "common.h"
#include "x86/x86.h"
#include "x86/memory.h"
#include "progress/progress.h"

#define NR_PUDIR 4096

PTE *get_ptab();

/*
static PDE pudir[NR_PROCESS][NR_PUDIR] __attribute((aligned(PAGE_SIZE)));
*/

/* main target is to access all memory */
uint32_t mm_alloc(uint32_t vaddr)
{
	return 0;
}
