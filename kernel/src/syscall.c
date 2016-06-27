#include "common.h"
#include "x86/x86.h"

#define NR_SYSCALL 32
typedef int(* SYSCALL_FUN)(TrapFrame *tf);

int ask(TrapFrame *tf);
void abort(const char *file, int line);

int shell(TrapFrame *tf);

int sem_init(TrapFrame *tf);
int sem_destroy(TrapFrame *tf);

int sem_wait(TrapFrame *tf);
int sem_post(TrapFrame *tf);

int pthread_create(TrapFrame *tf);
int pthread_join(TrapFrame *tf);

int open(TrapFrame *tf);
int close(TrapFrame *tf);
int read(TrapFrame *tf);
int write(TrapFrame *tf);
int lseek(TrapFrame *tf);

int list(TrapFrame *tf);

int fork(TrapFrame *tf);
int sleep(TrapFrame *tf);
int exit_thread(TrapFrame *tf);
int sys_printc(TrapFrame *tf);
int sys_prints(TrapFrame *tf);
int flip_screen(TrapFrame *tf);
int read_key(TrapFrame *tf);
int get_ms(TrapFrame *tf);
int user_abort(TrapFrame *tf)
{
	printk("\033[1;31mUser abort!\n\033[0m");
	abort((char *)(tf->ebx), tf->ecx);
	return -1;
}

int inv(TrapFrame *tf)
{
	printk("\033[1;31mUnhandled syscall : #%d at 0x%x\n\033[0m", tf->eax, tf->eip);
	assert(0);
	return 0;
}

SYSCALL_FUN syscall_fun[NR_SYSCALL] = 
{
/* 00 */ flip_screen, read_key, get_ms, user_abort,
/* 04 */ exit_thread, inv, fork, inv,
/* 08 */ sys_printc, sys_prints, sleep, inv,
/* 0b */ pthread_create, pthread_join, sem_wait, sem_post,
/* 10 */ sem_init, sem_destroy, inv, inv,
/* 14 */ open, close, read, write,
/* 18 */ list, lseek, shell, ask,
/* 1a */ inv, inv, inv, inv
};

void do_syscall(TrapFrame *tf)
{
	int sysid = ((tf->eax) & 0x1f);
	tf->eax = syscall_fun[sysid](tf);
}
