#ifndef __MEMOTY_H__
#define __MEMORY_H__

#define PAGE_SIZE 4096
#define PHY_SIZE (1 << 27)

#define NR_PDE (1 << 10)
#define NR_PTE (PHY_SIZE / PAGE_SIZE)

#define NR_PD_PT 10
#define NR_PT_MEM 12
#define NR_PD_MEM (NR_PD_PT + NR_PT_MEM)

#define PD_PT_SIZE (1 << NR_PD_PT)
#define PT_MEM_SIZE (1 << NR_PT_MEM)
#define PD_MEM_SIZE (1 << NR_PD_MEM)

static inline uint32_t make_pde(uint32_t addr)
{
	return ((addr & 0xfffff000) | 0x7);
}

static inline uint32_t make_pte(uint32_t addr)
{
	return ((addr & 0xfffff000) | 0x7);
}


#endif
