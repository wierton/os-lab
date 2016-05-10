#ifndef __THREAD_H__
#define __THREAD_H__

#define NR_THREAD 64
#define NULL_PTID -1
#define USER_KSTACK_SIZE 4096

/* time lock */
typedef struct {
	uint32_t tid, dirty;
	int tartime;
} TIMELOCK;

/* thread state */
enum {TS_UNALLOCED, TS_RUN, TS_WAIT, TS_BLOCKED};
/* ThreadPriority */
/*				driver          user  idle  */
typedef enum {TP_0, TP_1, TP_2, TP_3, TP_4, TP_MIN = 9999} ThreadPriority;

typedef struct tagTCB {
	TrapFrame tf;
	uint32_t kesp;
	uint8_t stack[USER_KSTACK_SIZE];
	ThreadPriority tp;/* 0: max ... 3: min */
	int tid, ptid, ppid, state, timescales, tartime;
	struct tagTCB *next;
} TCB;

typedef struct {
	uint32_t entry;
	ThreadPriority thread_prior;
	int ptid;
} ThreadAttr;

HANDLE create_thread(HANDLE hProc, ThreadAttr *pta);
void enter_thread(HANDLE hThread);
int block(TrapFrame *tf, HANDLE hThread);
int wakeup(TrapFrame *tf, HANDLE hThread);

#endif
