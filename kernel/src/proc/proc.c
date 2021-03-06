#include "common.h"
#include "string.h"
#include "x86/x86.h"
#include "proc/elf.h"
#include "proc/proc.h"
#include "x86/memory.h"
#include "device/fs.h"

#define USR_STACK_SIZE 4096

HANDLE apply_udir();
PDE *get_udir(HANDLE);
PDE *load_udir(HANDLE);

extern uint32_t cur_proc;

void copy_thread_tree(HANDLE hSrc, HANDLE hDst);
void copy_phypage(HANDLE hSrc, HANDLE hDst, uint32_t vaddr);
HANDLE mm_alloc(HANDLE hProcess, uint32_t vaddr, uint32_t size);
void update_tf(HANDLE hThread, TrapFrame *tf);

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
		pcb[i].pid = i;
		pcb[i].ppid = -1;
		pcb[i].timescales = 0;
		pcb[i].state = PS_UNALLOCED;
		pcb[i].pudir = NULL;
		pcb[i].pm_num = 0;
		pcb[i].pm = NULL;
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

HANDLE create_proc(char *path, ProcAttr *pa)
/* input argument should be unsigned char *path */
{
	/* every process should be treated as thread(main thread)
	 * thread is unit for procedure scheduling
	 * create_thread(hProc, entry, priority) */
	
	/* apply available process handle */
	HANDLE hProc = apply_ph();

	/* user stack addr */
	pcb[hProc].available_stack_addr = USER_STACK_ADDR;

	/* init state of cur_proc */
	pcb[hProc].timescales = 0;
	pcb[hProc].ppid = 0;

	/* load this thread's page directory */
	PDE *old_pdir = load_udir(hProc);
	uint32_t eip = load_elf(hProc, path);

	/* reload old pdir */
	load_dir(old_pdir);

	/* create main thread for this process */
	ThreadAttr ta;
	ta.entry = eip;
	ta.thread_prior = pa->proc_prior;
	ta.ptid = NULL_PTID;
	pcb[hProc].hMainThread = create_thread(hProc, &ta);

	return hProc;
}

uint32_t apply_stack_addr(HANDLE hProc, uint32_t size)
{
	assert(hProc < NR_PROCESS);
	pcb[hProc].available_stack_addr += (4 * 1024 + size);
	assert(pcb[hProc].available_stack_addr < 0xc0000000);
	mm_alloc(hProc, pcb[hProc].available_stack_addr - size, size);
	return pcb[hProc].available_stack_addr;
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
	mm_alloc(hDst, USER_STACK_ADDR - USER_STACK_SIZE, USER_STACK_SIZE);

	for(i = 0; i < pcb[hSrc].pm_num; i++)
	{
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
	update_tf(pcb[cur_proc].hMainThread, tf);

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

PCB * get_pcb(HANDLE hProc)
{
	assert(hProc < NR_PROCESS);
	return &pcb[hProc];
}

void destroy_proc(HANDLE hProc)
{
	assert(hProc < NR_PROCESS);
	pcb[hProc].ppid = -1;
	pcb[hProc].timescales = 0;
	pcb[hProc].state = PS_UNALLOCED;
	pcb[hProc].pudir = NULL;
	pcb[hProc].pm_num = 0;
	pcb[hProc].pm = NULL;
	free_memspace(hProc);
}

PHINFO get_memh(HANDLE hProc)
{
	assert(hProc < NR_PROCESS);
	PHINFO tmp;
	tmp.pm = pcb[hProc].pm;
	tmp.pm_num = pcb[hProc].pm_num;
	return tmp;
}
