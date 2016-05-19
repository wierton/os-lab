#include "common.h"
#include "lib.h"
#include "time.h"
#include "thread.h"
#include "semaphore.h"

#define BUF_SIZE 10

sem_t sem_free, sem_full;
int buflen = 0;

void *producer(void * args)
{
	while(1)
	{
		sem_wait(&sem_free);
		buflen ++;
		sem_post(&sem_full);
		printf("producer:%d.\n", buflen);
		sleep(100);
	}
	return NULL;
}

void *consumer(void * args)
{
	while(1)
	{
		sem_wait(&sem_full);
		buflen --;
		printf("consumer:%d.\n", buflen);
		sem_post(&sem_free);
		sleep(33);
	}
	return NULL;
}

int main()
{
	printf("\033[1;31mTestcase Start.\033[0m\n");
	pthread_t tid;
	buflen = 0;
	printf("buf size: %d\n", BUF_SIZE);

	sem_init(&sem_free, 0, BUF_SIZE);
	sem_init(&sem_full, 0, 0);

	pthread_create(&tid, NULL, consumer, NULL);
	pthread_create(&tid, NULL, producer, NULL);
	pthread_create(&tid, NULL, producer, NULL);
	pthread_create(&tid, NULL, producer, NULL);
	pthread_create(&tid, NULL, producer, NULL);

	printf("main thread sleep.\n");
	sleep(100000);
	sem_destroy(&sem_free);
	sem_destroy(&sem_full);
	return 0;
}
