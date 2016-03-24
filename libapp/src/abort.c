#include "common.h"
#include "lib.h"

void abort(const char *file, int line)
{
	syscall(3, file, line);
}
