#include "common.h"
#include "lib.h"
#include "time.h"
#include "thread.h"

void * func(void * args)
{
	printf("Thread Start with allocated number %d.\n", ((uint32_t *)args)[0]);
	return NULL;
}

int main()
{
	printf("\033[1;31mTestcase Start.\033[0m\n");
	pthread_t tid;

	int i, args[10];
	for(i = 0; i < 10; i++)
	{
		args[i] = i;
		pthread_create(&tid, NULL, func, &args[i]);
		printf("child thread #%d created.\n", tid);
	}

	printf("main thread sleep.\n");
	sleep(10);
	return 0;
}
