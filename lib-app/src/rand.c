#include "common.h"

static int ps;

void srand(int seed)
{
	ps = seed;
}

int rand()
{
	ps = (25173 * ps + 13849) & 0xffff;
	return ps;
}
