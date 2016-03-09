#include "common.h"

static int ps;

void srand(int seed)
{
	ps = seed;
}

int rand()
{
	ps = 134775813 * ps + 1;
	return ps;
}
