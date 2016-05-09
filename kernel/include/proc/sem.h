#ifndef __SEM_H__
#define __SEM_H__

#ifndef __PROC_H__
#include "proc.h"
#endif

typedef struct tagWLIST {
	HANDLE hThread;
	struct tagWLIST *next;
} WLIST;

typedef struct {
	int value;
	WLIST *wlist;
} sem_t;

#endif
