#include "common.h"
#include "string.h"
#include "x86/x86.h"
#include "proc/elf.h"
#include "proc/proc.h"

#define USR_STACK_SIZE 4096

HANDLE apply_udir();
PDE *get_udir(HANDLE);
PDE *load_udir(HANDLE);

static PCB pcb[NR_PROCESS];

void init_proc()
{
	int i;
	for(i = 0; i < NR_PROCESS; i++)
	{
		pcb[i].state = PS_UNALLOCED;
	}
}

/* apply process handle */
HANDLE apply_ph()
{
	HANDLE hProc = apply_udir();
	assert(hProc != INVALID_HANDLE_VALUE);

	pcb[hProc].pudir = get_udir(hProc);
	return hProc;
}

HANDLE create_proc(uint32_t disk_off)
/* input argument should be unsigned char *path */
{
	/* every process should be treated as thread(main thread)
	 * thread is unit for procedure scheduling
	 * create_thread(hProc, entry, priority) */
	
	/* apply available process handle */
	HANDLE hProc = apply_ph();

	/* load this thread's page directory */
	PDE *old_pdir = load_udir(hProc);
	uint32_t eip = load_elf(hProc, disk_off);

	/* reload old pdir */
	load_dir(old_pdir);

	/* create main thread for this process */
	pcb[hProc].hMainThread = create_thread(hProc, eip, TP_3);

	return hProc;
}

/* run this process(load main thread) */
void enter_proc(HANDLE hProc)
{
	/* enter main thread */
	load_udir(hProc);
	pcb[hProc].state = PS_RUN;
	load_udir(hProc);
	enter_thread(pcb[hProc].hMainThread);
}
