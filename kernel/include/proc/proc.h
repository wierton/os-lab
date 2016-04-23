#ifndef __PROCESS_H__
#define __PROCESS_H__

#include "proc/thread.h"

#define NR_PROCESS 32

#define USER_STACK_ADDR 0x50000000
#define USER_STACK_SIZE (1 << 22)

/* proc state */
enum {PS_RUN, PS_WAIT, PS_BLOCKED, PS_UNALLOCED};

typedef struct tagPCB {
	int pid, ppid, timescales, state;
	PDE *pudir;
	HANDLE hMainThread;
} PCB;

HANDLE apply_ph();
void pcb_time_plus(HANDLE hProc);
HANDLE create_proc(uint32_t disk_off);
void enter_proc(HANDLE hProc);
void switch_proc(TrapFrame *tf);

#endif
