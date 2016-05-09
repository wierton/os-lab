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
		sleep(200);
	}
	return NULL;
}

void *consumer(void * args)
{
	while(1)
	{
		sem_wait(&sem);
		printf("consumer:%d.\n", sem.value);
		sleep(133);
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
	while(1);
	return 0;
}
