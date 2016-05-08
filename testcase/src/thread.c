#include "common.h"
#include "lib.h"
#include "time.h"
#include "thread.h"

void * func(void * args)
{
	printf("Thread Start.\n");
	return NULL;
}

int main()
{
	printf("\033[1;31mTestcase Start.\033[0m\n");
	pthread_t tid;
	pthread_create(&tid, NULL, func, NULL);
	sleep(10);
	return 0;
}
