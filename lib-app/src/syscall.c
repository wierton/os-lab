#include "common.h"
#include "lib.h"

int __attribute__((noinline)) syscall(int ID, ...)
{
	int ret, *args = &ID;
	asm volatile("int $0x80":"=a"(ret):"a"(args[0]), "b"(args[1]), "c"(args[2]), "d"(args[3]));
	return ret;
}
