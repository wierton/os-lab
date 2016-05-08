#include "common.h"
#include "syscall.h"
#include "thread.h"
#include "lib.h"

int fork()
{
	return syscall(6);
}

void *__ThreadFunc(void * (*start_routine)(void *), void * args)
{
	start_routine(args);
	syscall(4);
	return NULL;
}

int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine) (void *), void *args)
{
	uint32_t hThread = syscall(12, __ThreadFunc, start_routine, args);
	*thread = hThread;
	return 0;
}

int pthread_join(pthread_t thread, void **retval)
{
	return 0;
}
