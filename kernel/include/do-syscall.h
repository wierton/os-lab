#ifndef __DO_SYSCALL_H__
#define __DO_SYSCALL_H__

void abort(const char *file, int line);

int fork(TrapFrame *tf);
int sleep(TrapFrame *tf);
int sys_printc(TrapFrame *tf);
int sys_prints(TrapFrame *tf);
int flip_screen(TrapFrame *tf);
int read_key(TrapFrame *tf);
int timer_event(TrapFrame *tf);
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

#endif
