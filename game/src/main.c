#include "common.h"

void init_serial();
void init_video();
void init_disk();
void test_printk();

int main()
{
	init_serial();

	init_video();

	init_disk();
	//test_printk();

	while(1);
	return 0;
}
