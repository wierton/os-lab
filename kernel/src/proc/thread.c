#include "common.h"
#include "string.h"
#include "x86/x86.h"
#include "x86/memory.h"
#include "proc/elf.h"
#include "proc/proc.h"
#include "device/time.h"

#define Q_WAIT  1
#define Q_BLOCK 2
#define NR_TIMELOCK 16
//#define DEBUG


char *ask_queue[] = {"none", "wait", "block"};
char *ask_ts[] = {"none", "run", "wait", "block"};

int shell(TrapFrame *);
static TCB tcb[NR_THREAD], *tq_wait = NULL, *tq_blocked = NULL;

uint32_t cur_esp;
uint32_t cur_thread = 0xffffffff, cur_proc = 0xffffffff;
uint32_t tmp_stack[4096];

TIMELOCK timelock[NR_TIMELOCK];

int debug_times = 0;
int sleep_times = 0;

void __attribute__((noinline)) debug()
{
	debug_times ++;
	printk("\033[1;32mdebug start\n\033[0m");
}

void __attribute__((noinline)) double_debug()
{
	printk("\033[1;32mdouble debug start\n\033[0m");
}

/* function defined from here */

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

	printk("%s chain(state, tid, ppid):", id ? "blocked" : "wait");
	while(tmp != NULL)
	{
		printk("(%s, %d, %d) -- ", ask_ts[tmp->state], tmp->tid, tmp->ppid);
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

#ifdef DEBUG
#define rm_queue(a, b) do { printk("%s,%d,%s(%d,%s):\n", __func__, __LINE__, "rm_queue", a, ask_queue[b]); \
	show_queue(-1);show_queue(0);show_queue(1);\
	rm_queue(a,b);\
	show_queue(-1);show_queue(0);show_queue(1);\
	printk("\n\n");\
}while(0)
#endif

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
		printk("%x %x %x\n", hThread, tcb[hThread].state, ts_tar);
		assert(0);
	}

	/* remove from rm queue */
	rm_queue(hThread, q_rm);
	tcb[hThread].state = ts_tar;

	/* add to tar queue */
	if(*ptar == NULL || (*ptar)->tp > tcb[hThread].tp)
	{
		tcb[hThread].next = *ptar;
		*ptar = &(tcb[hThread]);
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

	rm_queue(hThread, Q_WAIT);
	rm_queue(hThread, Q_BLOCK);
}

#ifdef DEBUG
#define add_queue(a, b) do { printk("%s,%d,%s(%d,%s):\n", __func__, __LINE__, "add_queue", a, ask_queue[b]);\
	show_queue(-1);show_queue(0);show_queue(1);\
   	add_queue(a,b);\
	show_queue(-1);show_queue(0);show_queue(1);\
	printk("\n\n");\
}while(0)

#define add_run(a) do { printk("%s,%d,%s(%d):\n", __func__, __LINE__, "add_run", a);\
	show_queue(-1);show_queue(0);show_queue(1);\
	add_run(a);\
	show_queue(-1);show_queue(0);show_queue(1);\
	printk("\n\n");\
}while(0)
#endif

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

	if(cur_thread != 0xffffffff && tcb[cur_thread].state == TS_RUN)
	{
		add_queue(cur_thread, Q_WAIT);
	}

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

void check_timelock()
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
		else if(tcb[timelock[i].tid].state == TS_UNALLOCED)
		{
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
	if(cur_thread == 0xffffffff)
	{
		if(tq_wait != NULL)
		{
			int tid = tq_wait->tid;
			add_run(tid);
			memcpy(tf, &(tcb[cur_thread].tf), sizeof(TrapFrame));
			load_udir(tcb[cur_thread].ppid);
			return cur_thread;
		}
		else
			assert(0);
	}

	uint32_t cur_tp = (uint32_t)TP_MIN;
	HANDLE new_thread = cur_thread;

	tcb[cur_thread].timescales ++;
	pcb_time_plus(cur_proc);
	cur_tp = tcb[cur_thread].tp;

	if(tcb[cur_thread].state == TS_BLOCKED)
		cur_tp = (uint32_t)TP_MIN;

	check_timelock();

	TCB *tmp = tq_wait;

	/* no thread left */
	if(tmp == NULL && tcb[cur_thread].state == TS_BLOCKED)
		assert(0);

	/* check for new thread which is prior to old thread */
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
		/* switch_thread call by sleep will result in cur_state != TS_RUN */
		if(tcb[old_thread].state == TS_RUN)
			add_queue(old_thread, Q_WAIT);
		add_run(new_thread);
	
		/* save TrapFrame information */
		memcpy(&(tcb[old_thread].tf), tf, sizeof(TrapFrame));
		memcpy(tf, &(tcb[new_thread].tf), sizeof(TrapFrame));
	}
	
	load_udir(tcb[new_thread].ppid);

	return new_thread;
}

#ifdef DEBUG
#define switch_thread(a) do { printk("%s,%d:\n", __func__, __LINE__);\
	void print_tf(TrapFrame *);\
	switch_thread(a);\
	print_tf(a);\
}while(0)
#endif

int sleep(TrapFrame *tf)
{
	assert(cur_thread != 0xffffffff);

	uint32_t p_sb = apply_timelock();
	timelock[p_sb].tid = cur_thread;
	timelock[p_sb].tartime = time() + tf->ebx;

	tcb[cur_thread].tartime = time() + tf->ebx;
	add_queue(cur_thread, Q_BLOCK);
	/* can't set cur_thread to 0xffffffff to avoid trapframe
	 * crash in first part of switch_thread */
	switch_thread(tf);
	/* if A call other syscall and interupted by B before saving
	 * return value , then B call sleep, sleep call switch
	 * thread and resume A's context, but after sleep return back
	 * to do_syscall, do_syscall will update eax by the value
	 * it got, so here wrote by tf->eax othrewise 0 */
	return tf->eax;
}

int block(TrapFrame *tf, HANDLE hThread)
{
	assert(hThread < NR_THREAD);
	add_queue(hThread, Q_BLOCK);
	switch_thread(tf);
	return 0;
}

int wakeup(TrapFrame *tf, HANDLE hThread)
{
	assert(hThread < NR_THREAD);
	add_queue(hThread, Q_WAIT);
	return 0;
}

void destroy_thread(HANDLE hThread)
{
	assert(hThread < NR_THREAD);

	rm_queue(hThread, Q_BLOCK);
	rm_queue(hThread, Q_WAIT);

	tcb[hThread].ptid = -1;
	tcb[hThread].ppid = -1;
	tcb[hThread].state = TS_UNALLOCED;
	tcb[hThread].timescales = 0;
	tcb[hThread].tartime = -1;
	tcb[hThread].next = NULL;
	tcb[hThread].kesp = (uint32_t)(tcb[hThread].stack) + USER_KSTACK_SIZE;
}

void block_curt()
{
	if(cur_thread == 0xffffffff)
		return;
	add_queue(cur_thread, Q_WAIT);
	cur_thread = 0xffffffff;
	cur_proc = 0xffffffff;
}

int exit_thread(TrapFrame *tf)
{
	int i;
	HANDLE ctid = cur_thread;
	HANDLE mtid = get_pcb(tcb[cur_thread].ppid)->hMainThread;
	HANDLE ppid = tcb[cur_thread].ppid;
	if(cur_thread == mtid)
	{
		/* destroy all threads belongs to this process */
		for(i = 0; i < NR_THREAD; i++)
		{
			if(tcb[i].state != TS_UNALLOCED && tcb[i].ppid == ppid)
				destroy_thread(i);
		}

		/* destroy process */
		destroy_proc(ppid);
		cur_proc = 0xffffffff;
	}
	else
	{
		destroy_thread(ctid);
	}

	cur_thread = 0xffffffff;
	switch_thread(tf);
	return tf->eax;
}

int pthread_create(TrapFrame *tf)
{
	ThreadAttr ta;
	ta.ptid = cur_thread;
	ta.entry = tf->ebx;
	ta.thread_prior = tcb[get_pcb(cur_proc)->hMainThread].tp;
	ta.ptid = cur_proc;
	HANDLE hThread = create_thread(cur_proc, &ta);
	uint32_t stack_top = apply_stack_addr(cur_proc, 4096) - 12;

	/* push args in stack */
	tcb[hThread].tf.esp = stack_top;
	((uint32_t *)(stack_top + 4))[0] = tf->ecx;
	((uint32_t *)(stack_top + 8))[0] = tf->edx;
	return hThread;
}

int pthread_join(TrapFrame *tf)
{
	return 0;
}


int ask(TrapFrame *tf)
{
	int i;
	if(tcb[cur_thread].tp < 4)
	{
		return 0;	
	}
	for(i = 0; i < NR_THREAD; i++)
	{
		if(tcb[i].state != TS_UNALLOCED && tcb[i].tp < 4)
		{
			return -1;
		}
	}

	return 1;
}

/* new syscall:
 * 1. fork()
 * 2. pthread_t(FuncEntry)
 * 3. join() // block current thread
 * 4. wait(hThread)
 * 5. sleep(ms);
 * 6. exit();
 */
