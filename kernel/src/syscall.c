#include "common.h"
#include "x86/x86.h"
#include "do-syscall.h"

#define NR_SYSCALL 32
typedef int(* SYSCALL_FUN)(TrapFrame *tf);

SYSCALL_FUN syscall_fun[NR_SYSCALL] = 
{
/* 00 */ flip_screen, read_key, time, user_abort,
/* 04 */ inv, inv, inv, inv,
/* 08 */ inv, inv, inv, inv,
/* 0b */ inv, inv, inv, inv,
/* 10 */ inv, inv, inv, inv,
/* 14 */ inv, inv, inv, inv,
/* 18 */ inv, inv, inv, inv,
/* 1a */ inv, inv, inv, inv
};

void do_syscall(TrapFrame *tf)
{
	int sysid = ((tf->eax) & 0x1f);
	tf->eax = syscall_fun[sysid](tf);
}