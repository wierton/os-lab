#include "common.h"
#include "lib.h"
#include "time.h"
#include "thread.h"

int main()
{
	printf("\033[1;31mTestcase Start.\033[0m\n");
	int ret = fork();
	int i = 0;
	if(ret == -1)
	{
		while(1)
		{
			printf("I'm child process, Ping!\n");
			while(i ++ < 29999999);
			i = 0;
		}
	}
	else
	{
		while(1)
		{
			printf("I'm parent process, Pong!\n");
			while(i ++ < 9999999);
			i = 0;
		}
	}
	return 0;
}
