#include "common.h"
#include "lib.h"

int main()
{
	int i = 0;
	while(1)
	{
		printf("Hello World!\n");
		while(i ++ < 99999999);
		i = 0;
	}
	return 0;
}
