#include "common.h"
#include "syscall.h"

uint32_t get_ms()
{
	return syscall(2);
}

void delay(uint32_t ms)
{
	uint32_t target = ms + get_ms();
	while(get_ms() < target);
}
