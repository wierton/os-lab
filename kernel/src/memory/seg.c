#include "common.h"
#include "x86/x86.h"
#include "x86/memory.h"
#include "progress/progress.h"

/* since game can't access memory above 0xc0000000, this is necessary */

void init_segment()
{
	return;
}
