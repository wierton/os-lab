#include "common.h"
#include "lib.h"
#include "thread.h"

int main()
{
	int i = 0;
	if(fork() == -1)
	{
		while(1)
		{
			printf("Ping!\n");
			while(i ++ < 1000000);
			i = 0;
		}
	}
	else
	{
		while(1)
		{
			printf("Pong!\n");
			while(i ++ < 1000000);
			i = 0;
		}
	}
	return 0;
}
