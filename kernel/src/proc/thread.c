#include "common.h"
#include "string.h"
#include "x86/x86.h"
#include "proc/elf.h"
#include "proc/proc.h"
#include "device/time.h"

#define Q_WAIT  1
#define Q_BLOCK 2

static TCB tcb[NR_THREAD], *tq_wait = NULL, *tq_blocked = NULL;

HANDLE apply_udir();
PDE *get_udir(HANDLE);
PDE *load_udir(HANDLE);
HANDLE get_mainthread(HANDLE hProc);
void destroy_proc(HANDLE hProc);

uint32_t cur_esp;
uint32_t cur_thread, cur_proc;
uint32_t tmp_stack[4096];

typedef struct {
	uint32_t tid, dirty;
	int tartime;
} TIMELOCK;

#define NR_TIMELOCK 16

TIMELOCK timelock[NR_TIMELOCK];

uint32_t apply_timelock()
{
	int i;
	for(i = 0; i < NR_TIMELOCK; i++)
	{
		if(!timelock[i].dirty)
		{
			timelock[i].dirty = 1;
			return i;
		}
	}
	assert(0);
	return 0xffffffff;
}

void show_queue(int id)
{
	TCB *tmp = NULL;

	if(id == -1)
	{
		printk("running: (%d, %d)\n", cur_thread, cur_proc);
		return;
	}

	if(id == 0)
		tmp = tq_wait;
	else
		tmp = tq_blocked;

	printk("%s chain(tid, ppid):", id ? "blocked" : "wait");
	while(tmp != NULL)
	{
		printk("(%d, %d, %d) -- ", tmp->state, tmp->tid, tmp->ppid);
		tmp = tmp->next;
	}
	printk("\n");
}

void show_tid()
{
	int i = 0;
	for(i = 0; i < 3; i++)
	{
		printk("/%d, %d, %d/", i, tcb[i].tid, tcb[i].ppid);
	}
	printk("\n");
}

void rm_queue(HANDLE hThread, uint32_t queue)
{
	TCB **prm = NULL;
	switch(queue)
	{
		case Q_WAIT:
			prm = &tq_wait;
			break;
		case Q_BLOCK:
			prm = &tq_blocked;
			break;
		default:assert(0);break;
	}

	assert(hThread < NR_THREAD);

	/* remove from rm queue */
	if(*prm == &tcb[hThread])
	{
		*prm = tcb[hThread].next;
	}
	else if(*prm != NULL)
	{
		TCB *tmp = *prm;
		while(tmp->next != NULL && tmp->next != &tcb[hThread])
		{
			tmp = tmp->next;
		}

		if(tmp->next != NULL)
			tmp->next = tcb[hThread].next;
	}
}

void add_queue(HANDLE hThread, uint32_t queue)
{
	TCB **ptar = NULL;
	uint32_t ts_tar = -1, q_rm = -1;
	switch(queue)
	{
		case Q_WAIT:
			ptar = &tq_wait;
			q_rm = Q_BLOCK;
			ts_tar = TS_WAIT;
			break;
		case Q_BLOCK:
			ptar = &tq_blocked;
			q_rm = Q_WAIT;
			ts_tar = TS_BLOCKED;
			break;
		default:assert(0);break;
	}

	assert(hThread < NR_THREAD);

	if(tcb[hThread].state == ts_tar)
	{
		assert(0);
	}

	/* remove from rm queue */
	rm_queue(hThread, q_rm);
	tcb[hThread].state = ts_tar;

	/* add to tar queue */
	if(*ptar == NULL)
	{
		*ptar = &(tcb[hThread]);
		tcb[hThread].next = NULL;
	}
	else
	{
		TCB *tmp = *ptar;
		while(tmp->next != NULL && tmp->next->tp <= tcb[hThread].tp)
		{
			tmp = tmp->next;
		}

		tcb[hThread].next = tmp->next;
		tmp->next = &(tcb[hThread]);
	}
}

void add_run(HANDLE hThread)
{
	assert(hThread < NR_THREAD);
	/* record current context information */
	cur_thread = hThread;
	cur_proc = tcb[hThread].ppid;
	cur_esp = tcb[hThread].kesp;
	tcb[hThread].state = TS_RUN;

	/* remove from wait queue */
	TCB *tmp = tq_wait;
	if(tmp->tid == hThread)
	{
		tq_wait = tq_wait->next;
	}
	else
	{
		while(tmp != NULL && tmp->next->tid != hThread)
		{
			tmp = tmp->next;
		}

		if(tmp)
			tmp->next = tcb[hThread].next;
	}

	/* remove from blocked queue */
	tmp = tq_blocked;
	if(tmp->tid == hThread)
	{
		tq_blocked = tq_blocked->next;
	}
	else
	{
		while(tmp != NULL && tmp->next != &tcb[hThread])
		{
			tmp = tmp->next;
		}

		if(tmp)
			tmp->next = tcb[hThread].next;
	}

	tcb[hThread].next = NULL;
}

void init_thread()
{
	//TODO:init the thread control blocks
	cur_esp = (uint32_t)(tmp_stack) + 4096;

	tq_wait = tq_blocked = NULL;

	int i;
	for(i = 0; i < NR_THREAD; i++)
	{
		tcb[i].tid = i;
		tcb[i].ptid = -1;
		tcb[i].ppid = -1;
		tcb[i].state = TS_UNALLOCED;
		tcb[i].timescales = 0;
		tcb[i].tartime = -1;
		tcb[i].next = NULL;
		tcb[i].kesp = (uint32_t)(tcb[i].stack) + USER_KSTACK_SIZE;
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
	tcb[hThread].ptid = pta->ptid;
	tcb[hThread].ppid = hProc;
	tcb[hThread].timescales = 0;
	/* add this thread to wait queue */
	add_queue(hThread, Q_WAIT);

	/* construct pesudo trapframe */
	set_usrtf(pta->entry, &tcb[hThread].tf);
	
	return hThread;
}

/* run this thread */
void enter_thread(HANDLE hThread)
{
	assert(hThread < NR_THREAD);

	/* record current context information */
	add_run(hThread);
	
	void *tf = (void *)USER_STACK_ADDR - sizeof(TrapFrame);
	memcpy(tf, &tcb[hThread].tf, sizeof(TrapFrame));
	env_run(tf);
}

void update_tf(HANDLE hThread, TrapFrame *tf)
{
	assert(hThread < NR_THREAD);
	memcpy(&(tcb[hThread].tf), tf, sizeof(TrapFrame));
}

void check_block()
{
	int i;
	for(i = 0; i < NR_TIMELOCK; i++)
	{
		if(!timelock[i].dirty)
			continue;

		int t = time();
		if(t >= timelock[i].tartime && tcb[timelock[i].tid].state == TS_BLOCKED)
		{
			add_queue(timelock[i].tid, Q_WAIT);
			/* destroy timelock */
			timelock[i].dirty = 0;
		}
	}
}

void copy_thread_tree(HANDLE hSrc, HANDLE hDst)
{
	assert(cur_thread < NR_THREAD);

	int i;
	for(i = 0; i < NR_THREAD; i++)
	{
		if(tcb[i].ppid == hSrc)
		{
			HANDLE hnew = apply_th();
			tcb[hnew] = tcb[i];
			tcb[hnew].tid = hnew;
			tcb[hnew].ppid = hDst;
			tcb[hnew].kesp = (uint32_t)(tcb[hnew].stack) + USER_KSTACK_SIZE;
			if(tcb[i].state == TS_RUN || tcb[i].state == TS_WAIT)
			{
				add_queue(hnew, Q_WAIT);
			}
			else if(tcb[i].state == TS_BLOCKED)
			{
				add_queue(hnew, Q_BLOCK);
			}

			if(i == cur_thread)
			{
				tcb[hnew].tf.eax = -1;
				tcb[i].tf.eax = hDst;
			}
		}
	}
}

void __attribute__((noinline)) debug()
{
	printk("\033[1;32mdebug start\n\033[0m");
}

void show_tf(TrapFrame *tf)
{
	printk("v=%x, e=%x, eip=%x\n", tf->irq, tf->err, tf->eip);
	printk("eax:%x, ebx:%x, ecx:%x, edx:%x\n", tf->eax, tf->ebx, tf->ecx, tf->edx);
	printk("esi:%x, edi:%x, ebp:%x, esp:%x\n", tf->esi, tf->edi, tf->ebp, tf->esp);
}

HANDLE switch_thread(TrapFrame *tf)
{
	/* TODO: choose a thread from wait queue by priority
	 * save current thread's trapframe :
	 *		memcpy(cur_thread.tf, tf, sizeof(tf))
	 * load new thread's trapframe :
	 *		memcpy(tf, new_thread.tf, sizeof(tf))
	 * set cur_esp and cur_handle
	 */
	uint32_t cur_tp = 9999;
	HANDLE new_thread = cur_thread;

	if(cur_thread != 0xffffffff)
	{
		tcb[cur_thread].timescales ++;
		memcpy(&(tcb[cur_thread].tf), tf, sizeof(TrapFrame));
		pcb_time_plus(cur_proc);
		cur_tp = tcb[cur_thread].tp;
	}

	check_block();
	
	TCB *tmp = tq_wait;

	if(tmp == NULL && tcb[cur_thread].state ==TS_BLOCKED)
		poweroff();

	while(tmp != NULL)
	{
		if(tmp->tp <= cur_tp)
		{
			new_thread = tmp->tid;
			break;
		}
		tmp = tmp->next;
	}

	if(new_thread != cur_thread)
	{
		uint32_t old_thread = cur_thread;
		if(tcb[cur_thread].state == TS_RUN)
			add_queue(cur_thread, Q_WAIT);
		add_run(new_thread);

		/* store TrapFrame information */
		memcpy(&(tcb[old_thread].tf), tf, sizeof(TrapFrame));
		memcpy(tf, &(tcb[new_thread].tf), sizeof(TrapFrame));

		load_udir(tcb[new_thread].ppid);
	}

	return new_thread;
}


int sleep(TrapFrame *tf)
{
	assert(cur_thread != 0xffffffff);

	uint32_t p_sb = apply_timelock();
	timelock[p_sb].tid = cur_thread;
	timelock[p_sb].tartime = time() + tf->ebx;

	tcb[cur_thread].tartime = time() + tf->ebx;
	add_queue(cur_thread, Q_BLOCK);
	switch_thread(tf);
	return 0;
}

void destroy_thread(HANDLE hThread, TrapFrame *tf)
{
	assert(hThread < NR_THREAD);
	add_queue(hThread, Q_BLOCK);
	switch_thread(tf);
	assert(cur_thread != hThread);
	rm_queue(hThread, Q_BLOCK);

	tcb[hThread].ptid = -1;
	tcb[hThread].ppid = -1;
	tcb[hThread].state = TS_UNALLOCED;
	tcb[hThread].timescales = 0;
	tcb[hThread].tartime = -1;
	tcb[hThread].next = NULL;
	tcb[hThread].kesp = (uint32_t)(tcb[hThread].stack) + USER_KSTACK_SIZE;
}

int exit_thread(TrapFrame *tf)
{
	int i;
	HANDLE mtid = get_mainthread(tcb[cur_thread].ppid);
	HANDLE ppid = tcb[cur_thread].ppid;
	if(cur_thread == mtid)
	{
		/* destroy all threads belongs to this process */
		for(i = 0; i < NR_THREAD; i++)
		{
			if(tcb[i].state != TS_UNALLOCED && tcb[i].ppid == ppid)
			{
				destroy_thread(i, tf);
			}
		}

		/* destroy process */
		destroy_proc(ppid);
	}
	else
		destroy_thread(cur_thread, tf);

	return 0;
}

/* new syscall:
 * 1. fork()
 * 2. pthread_t(FuncEntry)
 * 3. join() // block current thread
 * 4. wait(hThread)
 * 5. sleep(ms);
 * 6. exit();
 */
