#include "common.h"
#include "string.h"
#include "x86/x86.h"
#include "proc/elf.h"
#include "proc/proc.h"

static TCB tcb[NR_THREAD], *tq_wait, *tq_blocked;

HANDLE apply_udir();
PDE *get_udir(HANDLE);
PDE *load_udir(HANDLE);

uint32_t cur_esp;
uint32_t cur_thread, cur_proc;
uint32_t tmp_stack[4096];

void add_wait(HANDLE hThread)
{
	assert(hThread < NR_THREAD);

	if(tcb[hThread].state == TS_UNALLOCED)
		assert(0);

	if(tcb[hThread].state == TS_WAIT)
		return;

	if(tcb[hThread].state == TS_BLOCKED)
	{
		/* remove from blocked queue */
		TCB *tmp = tq_blocked;
		assert(tmp != NULL);
		while(tmp->next != &tcb[hThread])
		{
			tmp = tmp->next;
		}

		tmp->next = tcb[hThread].next;
	}

	/* add to wait queue */
	tcb[hThread].state = TS_WAIT;
	tcb[hThread].next = tq_wait;
	tq_wait = &tcb[hThread];
}

void add_block(HANDLE hThread)
{
	assert(hThread < NR_THREAD);

	if(tcb[hThread].state == TS_UNALLOCED)
		assert(0);

	if(tcb[hThread].state == TS_BLOCKED)
		return;

	if(tcb[hThread].state == TS_WAIT)
	{
		/* remove from wait queue */
		TCB *tmp = tq_wait;
		assert(tmp != NULL);
		while(tmp->next != &tcb[hThread])
		{
			tmp = tmp->next;
		}

		tmp->next = tcb[hThread].next;
	}

	/* add to wait queue */
	tcb[hThread].state = TS_BLOCKED;
	tcb[hThread].next = tq_blocked;
	tq_blocked = &tcb[hThread];
}

void init_thread()
{
	//TODO:init the thread control blocks
	cur_esp = (uint32_t)(tmp_stack) + 4096;

	int i;
	for(i = 0; i < NR_THREAD; i++)
	{
		tcb[i].state = TS_UNALLOCED;
		tcb[i].next = NULL;
	}
}

/* apply thread handle */
HANDLE apply_th()
{
	int i;
	for(i = 0; i < NR_THREAD; i++)
	{
		if(tcb[i].state == TS_UNALLOCED)
			return i;
	}
	assert(0);
	return INVALID_HANDLE_VALUE;
}

HANDLE create_thread(HANDLE hProc, ThreadAttr *pta)
{
	/* apply a thread handle */
	HANDLE hThread = apply_th();

	/* init the tcb info */
	tcb[hThread].tf.eip = pta->entry;
	tcb[hThread].kesp = (uint32_t)(tcb[hThread].stack) + USER_KSTACK_SIZE;
	tcb[hThread].tp = pta->thread_prior;
	tcb[hThread].tid = hThread;
	tcb[hThread].ptid = pta->ptid;
	tcb[hThread].ppid = hProc;
	tcb[hThread].state = TS_WAIT;
	tcb[hThread].timescales = 0;

	/* construct pesudo trapframe */
	set_usrtf(pta->entry, &tcb[hThread].tf);

	/* add this thread to wait queue */
	add_wait(hThread);
	
	return hThread;
}

/* run this thread */
void enter_thread(HANDLE hThread)
{
	assert(hThread < NR_THREAD);

	/* record current context information */
	cur_thread = hThread;
	cur_proc = tcb[hThread].ppid;
	cur_esp = tcb[hThread].kesp;
	
	tcb[hThread].state = TS_RUN;
	void *tf = (void *)USER_STACK_ADDR - sizeof(TrapFrame);
	memcpy(tf, &tcb[hThread].tf, sizeof(TrapFrame));
	env_run(tf);
}

void switch_thread(TrapFrame *tf)
{
	/* TODO: choose a thread from wait queue by priority
	 * save current thread's trapframe :
	 *		memcpy(cur_thread.tf, tf, sizeof(tf))
	 * load new thread's trapframe :
	 *		memcpy(tf, new_thread.tf, sizeof(tf))
	 * set cur_esp and cur_handle
	 */

	tcb[cur_thread].timescales ++;
	pcb_time_plus(cur_proc);

	HANDLE new_thread = cur_thread;
	TCB *tmp = tq_wait;
	while(tmp != NULL)
	{
		if(tmp->tp > tcb[cur_thread].tp)
		{
			new_thread = tcb[cur_thread].tid;
			break;
		}
		tmp = tmp->next;
	}

	if(new_thread != cur_thread)
	{
		cur_proc = tcb[new_thread].ppid;
		memcpy(&(tcb[cur_thread].tf), tf, sizeof(TrapFrame));
		memcpy(tf, &(tcb[new_thread].tf), sizeof(TrapFrame));
	}
	
}

/* new syscall:
 * 1. fork()
 * 2. pthread_t(FuncEntry)
 * 3. join() // block current thread
 * 4. wait(hThread)
 * 5. sleep(ms);
 * 6. exit();
 */
