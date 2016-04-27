#include "common.h"
#include "syscall.h"
#include "thread.h"

int fork()
{
	return syscall(6);
}

int create_thread()
{
	return 0;
}
