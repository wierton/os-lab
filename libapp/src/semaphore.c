#include "common.h"
#include "syscall.h"
#include "semaphore.h"

sem_t * sem_open(const char *name, int oflag, mode_t mode, unsigned int value);
int sem_close(sem_t *sem);

int sem_init(sem_t *sem, int pshared, unsigned int value)
{
	return syscall(16, sem, value);
}

int sem_destroy(sem_t *sem)
{
	return syscall(17, sem);
}

/* P operation */
int sem_wait(sem_t *sem)
{
	return syscall(14, sem);
}

/* V operation */
int sem_post(sem_t *sem)
{
	return syscall(15, sem);
}
