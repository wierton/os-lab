#include "common.h"

static int ps;

void srand(int seed)
{
	ps = seed;
}

int rand()
{
	ps = 0;
	return ps;
}
