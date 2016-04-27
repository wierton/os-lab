#include "common.h"
#include "syscall.h"
#include "time.h"

void sleep(uint32_t ms)
{
	syscall(10, ms);
}
