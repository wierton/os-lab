#include "common.h"
#include "syscall.h"
#include "draw.h"

static uint8_t vbuf[SCR_BITSIZE];

int draw_rect(uint32_t color, int x, int y, uint32_t w, uint32_t h)
{
	if(x >= SCR_W || y >= SCR_H || x + w < 0 || y + h < 0)
		return 0;
	w = (x + w) >= SCR_W ? (SCR_W - 1 - x) : w;
	h = (y + h) >= SCR_H ? (SCR_H - 1 - y) : h;
	x = x < 0 ? 0 : x;
	y = y < 0 ? 0 : y;

	int i, j;
	for(i = x; i < x + w; i++)
		for(j = y; j < y + h; j++)
		{
			uint32_t *pixels = (void*)vbuf + 3 * i + j *SCR_BPL;
			pixels[0] = color;
		}

	return 0;
}

void invalidate()
{
	syscall(0, vbuf);
}
