#include "common.h"
#include "lib.h"

int main()
{
	int i = 0;
	while(1)
	{
		printf("idle process.\n");
		while(i ++ < 9999999);
		i = 0;
	}
	return 0;
}
