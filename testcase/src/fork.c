#include "common.h"
#include "lib.h"
#include "thread.h"

int main()
{
	printf("\033[1;31mGame Start.\033[0m\n");
	int i = 0, ret = fork();
	printf("game:{%d}\n", ret);
	if(ret == -1)
	{
		while(1)
		{
			printf("Ping!\n");
			while(i ++ < 1000000);
			i = 0;
			assert(0);
		}
	}
	else
	{
		while(1)
		{
			printf("Pong!\n");
			while(i ++ < 1000000);
			i = 0;
			assert(0);
		}
	}
	return 0;
}
