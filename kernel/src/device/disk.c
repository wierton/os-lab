#include "common.h"
#include "math.h"
#include "x86/x86.h"

void wait_disk()
{
	while((in_byte(0x1f7) & 0xc0) != 0x40);
}

void read_section(uint32_t dst, int sectnum)
{
	wait_disk();
	out_byte(0x1f2, 1);
	out_byte(0x1f3, sectnum);
	out_byte(0x1f4, sectnum >> 8);
	out_byte(0x1f5, sectnum >> 16);
	out_byte(0x1f6, (sectnum >> 24) | 0xE0);
	out_byte(0x1f7, 0x20);
	wait_disk();
	uint32_t tar = dst + 512;
	for(;dst < tar; dst += 4)
	{
		((uint32_t *)dst)[0] = in_long(0x1f0);
	}
}

void read_disk(uint32_t dst, uint32_t offset, uint32_t size)
{
	int i, op = 0, ed = 0;
	uint32_t off = offset / 512 + 9;
	uint32_t td = dst - offset % 512;
	uint32_t final = dst + size;

	uint8_t sect[512];
	for(; td < final; td += 512, off++)
	{
		read_section((uint32_t)sect, off);
		op = max(td, dst);
		ed = min(td + 512, final);
		for(i = op; i < ed; i++)
		{
			((uint8_t *)i)[0] = sect[i - td];
		}
	}
}

void init_disk()
{
	//read_section(0x8000, 0);
}
