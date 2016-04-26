#include "common.h"
#include "string.h"
#include "x86/x86.h"
#include "proc/elf.h"
#include "proc/proc.h"
#include "x86/memory.h"

#define USR_STACK_SIZE 4096

HANDLE apply_udir();
PDE *get_udir(HANDLE);
PDE *load_udir(HANDLE);

extern uint32_t cur_proc;

void copy_thread_tree(HANDLE hSrc, HANDLE hDst);
void copy_phypage(HANDLE hSrc, HANDLE hDst, uint32_t vaddr);
HANDLE mm_alloc(HANDLE hProcess, uint32_t vaddr, uint32_t size);
void update_eip(HANDLE hThread, uint32_t eip);

static PCB pcb[NR_PROCESS];

void pcb_time_plus(HANDLE hProc)
{
	assert(hProc < NR_PROCESS);
	pcb[hProc].timescales ++;
}

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

	/* init state of cur_proc */
	pcb[hProc].timescales = 0;
	pcb[hProc].ppid = 0;

	/* load this thread's page directory */
	PDE *old_pdir = load_udir(hProc);
	uint32_t eip = load_elf(hProc, disk_off);

	/* reload old pdir */
	load_dir(old_pdir);

	/* create main thread for this process */
	ThreadAttr ta;
	ta.entry = eip;
	ta.thread_prior = TP_3;
	ta.ptid = NULL_PTID;
	pcb[hProc].hMainThread = create_thread(hProc, &ta);

	return hProc;
}

/* run this process(load main thread) */
void enter_proc(HANDLE hProc)
{
	/* enter main thread */
	/* load process dir */
	load_udir(hProc);

	/* mark process state */
	pcb[hProc].state = PS_RUN;

	/* enter main thread of this process */
	enter_thread(pcb[hProc].hMainThread);
}


void copy_map(HANDLE hSrc, HANDLE hDst)
{
	assert(hSrc < NR_PROCESS && hDst < NR_PROCESS);

	int i;
	MEMH *pm = pcb[hSrc].pm;
	pcb[hDst].pm = pm;
	pcb[hDst].pm_num = pcb[hSrc].pm_num;
	printk("(%d)\n", pcb[hSrc].pm_num);
	for(i = 0; i < pcb[hSrc].pm_num; i++)
	{
		printk("[++:%x, %x],", pm[i].p_vaddr, pm[i].p_memsz);
		mm_alloc(hDst, pm[i].p_vaddr, pm[i].p_memsz);
	}
}

void copy_mem_deepin(HANDLE hSrc, HANDLE hDst)
{
	assert(hSrc < NR_PROCESS && hDst < NR_PROCESS);

	int i, j;
	MEMH *pm = pcb[hSrc].pm;

	mm_alloc(hDst, USER_STACK_ADDR - USER_STACK_SIZE, USER_STACK_SIZE);
	for(j = USER_STACK_ADDR - USER_STACK_SIZE; j < USER_STACK_ADDR; j += PAGE_SIZE)
	{
		copy_phypage(hSrc, hDst, j);
	}
	
	for(i = 0; i < pcb[hSrc].pm_num; i++)
	{
		mm_alloc(hDst, pm[i].p_vaddr, pm[i].p_memsz);
		uint32_t ex_bytes = pm[i].p_vaddr % PAGE_SIZE;
		uint32_t vaddr_op = pm[i].p_vaddr - ex_bytes;
		uint32_t vaddr_ed = pm[i].p_vaddr + pm[i].p_memsz;
		for(j = vaddr_op; j < vaddr_ed; j += PAGE_SIZE)
		{
			copy_phypage(hSrc, hDst, j);
		}
	}
}

int fork(TrapFrame *tf)
/* fork: copy memspace and page dir, page tab */
{
	/* apply available process handle */
	HANDLE hProc = apply_ph();
	update_eip(pcb[cur_proc].hMainThread, tf->eip);

	/* init state of cur_proc */
	pcb[hProc].timescales = 0;
	pcb[hProc].ppid = cur_proc;

	/* deepcopy */
	copy_map(cur_proc, hProc);
	copy_mem_deepin(cur_proc, hProc);
	copy_thread_tree(cur_proc, hProc);

	return cur_proc;
}

void set_memh(HANDLE hProc, uint32_t pm_num, MEMH *pm)
{
	assert(hProc < NR_PROCESS);
	
	pcb[hProc].pm_num = pm_num;
	pcb[hProc].pm = pm;
}

