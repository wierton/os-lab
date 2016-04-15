#ifndef __THREAD_H__
#define __THREAD_H__

#define NR_THREAD 64

#define USER_KSTACK_SIZE 4096

/* thread state */
enum {TS_UNALLOCED, TS_RUN, TS_WAIT, TS_BLOCKED};
/* ThreadPriority */
typedef enum {TP_0, TP_1, TP_2, TP_3} ThreadPriority;

typedef struct tagTCB {
	TrapFrame tf;
	uint8_t stack[USER_KSTACK_SIZE];
	ThreadPriority tp;/* 0: max ... 5: min */
	int tid, ptid, ppid, state, timescales;
	struct tagTCB *next;
} TCB;

HANDLE create_thread(HANDLE hProc, uint32_t entry, ThreadPriority tp);
void enter_thread(HANDLE hThread);

#endif
