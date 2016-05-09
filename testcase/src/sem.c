#include "common.h"
#include "lib.h"
#include "time.h"
#include "thread.h"
#include "semaphore.h"
	
sem_t sem;

void *producer(void * args)
{
	while(1)
	{
		sem_post(&sem);
		printf("producer:%d.\n", sem.value);
		sleep(100);
	}
	return NULL;
}

void *consumer(void * args)
{
	while(1)
	{
		sem_wait(&sem);
		printf("consumer:%d.\n", sem.value);
		sleep(33);
	}
	return NULL;
}

int main()
{
	printf("\033[1;31mTestcase Start.\033[0m\n");
	pthread_t tid;

	sem_init(&sem, 0, 0);
	pthread_create(&tid, NULL, consumer, NULL);
	pthread_create(&tid, NULL, producer, NULL);

	printf("main thread sleep.\n");
	sleep(10000);
	sem_destroy(&sem);
	return 0;
}
