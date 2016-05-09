#ifndef __SEMPHORE_H__
#define __SEMPHORE_H__

/* ERRORS */
#define EACCES			1
#define EEXIST			(1 << 1)
#define EINVAL			(1 << 2)
#define EMFILE			(1 << 3)
#define ENAMETOOLONG	(1 << 4)
#define ENFILE			(1 << 5)
#define ENOENT			(1 << 6)
#define ENOMEM			(1 << 7)

#define __SIZEOF_SEM_T 16

/* Value returned if `sem_open' failed.  */
#define SEM_FAILED      ((sem_t *) 0)

typedef struct {
	int value;
	void *wlist;
} sem_t;
typedef uint32_t mode_t;

/* treated as a shared file */
sem_t * sem_open(const char *name, int oflag, mode_t mode, unsigned int value);
int sem_close(sem_t *sem);

int sem_init(sem_t *sem, int pshared, unsigned int value);
int sem_destroy(sem_t *sem);

int sem_wait(sem_t *sem);
int sem_post(sem_t *sem);

#endif
