#include "common.h"
#include "lib.h"
#include "time.h"
#include "thread.h"

int main()
{
	printf("\033[1;31mTestcase Start.\033[0m\n");
	int ret = fork();
	if(ret == -1)
	{
		while(1)
		{
			printf("I'm child process, Ping!\n");
			sleep(100);
		}
	}
	else
	{
		while(1)
		{
			printf("I'm parent process, Pong!\n");
			sleep(100);
		}
	}
	return 0;
}
