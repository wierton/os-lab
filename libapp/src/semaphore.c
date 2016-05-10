#include "common.h"
#include "lib.h"
#include "syscall.h"
#include "semaphore.h"

#define NR_SEM 10

static int pe = 0;
static sem_t sem[NR_SEM];

sem_t * sem_open(const char *name, int oflag, mode_t mode, unsigned int value)
{
	assert(pe < NR_SEM);
	sem_init(&sem[pe ++], 0, value);
	return NULL;
}

int sem_close(sem_t *sem)
{
	sem_destroy(sem);
	return 0;
}

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
