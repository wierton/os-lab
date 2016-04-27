#ifndef __PROCESS_H__
#define __PROCESS_H__

#include "proc/thread.h"

#define NR_PROCESS 32

#define USER_STACK_ADDR 0x50000000
#define USER_STACK_SIZE (1 << 22)

/* proc state */
enum {PS_RUN, PS_WAIT, PS_BLOCKED, PS_UNALLOCED};

typedef struct {
	uint32_t p_vaddr, p_paddr, p_memsz;
} MEMH;

static inline MEMH make_memh(uint32_t x, uint32_t y, uint32_t z)
{
	MEMH tmp;
	tmp.p_vaddr = x;
	tmp.p_paddr = y;
	tmp.p_memsz = z;
	return tmp;
}

typedef struct tagPCB {
	int pid, ppid, timescales, state;
	PDE *pudir;
	HANDLE hMainThread;
	uint32_t pm_num;
	MEMH *pm;
} PCB;

typedef struct {
	uint32_t proc_prior;
} ProcAttr;

HANDLE apply_ph();
void pcb_time_plus(HANDLE hProc);
void enter_proc(HANDLE hProc);
void switch_proc(TrapFrame *tf);
HANDLE create_proc(uint32_t disk_off, ProcAttr *pa);

#endif
