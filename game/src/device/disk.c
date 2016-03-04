#include "common.h"

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
	uint32_t ed = dst + size;
	uint32_t off = offset / 512 + 2;
	dst -= (offset % 512);
	for(; dst < ed; dst += 512, off++)
	{
		read_section(dst, off);
	}
}

void init_disk()
{
	//read_section(0x8000, 0);
}
