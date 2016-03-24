#include "common.h"
#include "syscall.h"

uint32_t read_key()
{
	return syscall(1);
}
