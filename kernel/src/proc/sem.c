#include "common.h"
#include "string.h"
#include "x86/x86.h"
#include "proc/elf.h"
#include "proc/sem.h"
#include "proc/proc.h"
#include "device/time.h"

#define NR_WLIST 64

static WLIST wlist[NR_WLIST], *wfree;

extern uint32_t cur_thread;

void init_wlist()
{
	int i = 0;
	for(i = 0; i < NR_WLIST - 1; i++)
	{
		wlist[i].next = &wlist[i + 1];
	}
	wlist[i].next = NULL;
	wfree = &wlist[0];
}

WLIST *apply_wlist()
{
	assert(wfree != NULL);
	WLIST *tmp = wfree;
	wfree = wfree->next;
	tmp->next = NULL;
	return tmp;
}

void free_wlist(WLIST *wf)
{
	wf->next = wfree;
	wfree = wf;
}

int sem_init(TrapFrame *tf)
{
	sem_t *sem = (void *)tf->ebx;
	sem->value = tf->ecx;
	sem->wlist = NULL;
	return 0;
}

int sem_destroy(TrapFrame *tf)
{
	sem_t *sem = (void *)tf->ebx;
	WLIST *wf = sem->wlist;
	sem->wlist = NULL;
	while(wf != NULL)
	{
		WLIST *tmp = wf;
		wf = wf->next;
		free_wlist(tmp);
	}
	return 0;
}

/* P operation */
int sem_wait(TrapFrame *tf)
{
	sem_t *sem = (void *)(tf->ebx);
	sem->value --;
	if(sem->value < 0)
	{
		WLIST *tmp = apply_wlist();
		tmp->next = sem->wlist;
		tmp->hThread = cur_thread;
		sem->wlist = tmp;
		block(tf, cur_thread);
	}
	return 0;
}

/* V operation */
int sem_post(TrapFrame *tf)
{
	sem_t *sem = (void *)(tf->ebx);
	sem->value ++;
	if(sem->value <= 0)
	{
		if(sem->wlist != NULL)
		{
			wakeup(tf, sem->wlist->hThread);
		}
		WLIST *tmp = sem->wlist;
		sem->wlist = tmp->next;
		free_wlist(tmp);
	}
	return 0;
}
