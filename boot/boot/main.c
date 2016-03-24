#include "common.h"

void wait_disk()
{
	//the byte read from 0x1f7 port with bits 0B01xxxxxx indicate the disk is prepared
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

void boot_main()
{
	int i;
	uint32_t dst = 0x9000;
	for(i = 1; i < 9; i++)
	{
		read_section(dst, i);
		dst += 512;
	}
	asm volatile("push %0;ret"::"a"(0x9000));
}
