#include "common.h"
#include "math.h"
#include "x86/x86.h"

#define SECTSIZE 512

#define IDE_BSY		0x80
#define IDE_DRDY	0x40
#define IDE_DF		0x20
#define IDE_ERR		0x01

static int diskno = 1;

void __attribute__((noinline)) wait_disk()
{
	while((in_byte(0x1f7) & 0xc0) != 0x40);
}

static int ide_wait_ready(bool check_error)
{
	int r;

	while (((r = in_byte(0x1F7)) & (IDE_BSY|IDE_DRDY)) != IDE_DRDY);

	if (check_error && (r & (IDE_DF|IDE_ERR)) != 0)
		return -1;
	return 0;
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

void read_disk(void *dst, uint32_t offset, uint32_t size)
{
	int i, op = 0, ed = 0;
	uint32_t off = offset / 512;
	uint32_t td = (uint32_t)dst - offset % 512;
	uint32_t final = (uint32_t)dst + size;

	uint8_t sect[512];
	for(; td < final; td += 512, off++)
	{
		read_section((uint32_t)sect, off);
		op = max(td, (uint32_t)dst);
		ed = min(td + 512, final);
		for(i = op; i < ed; i++)
		{
			((uint8_t *)i)[0] = sect[i - td];
		}
	}
}


int write_section(uint32_t secno, const void *src, size_t nsecs)
{
	int r = 0;
	assert(nsecs <= 256);

	ide_wait_ready(0);

	out_byte(0x1F2, nsecs);
	out_byte(0x1F3, secno & 0xFF);
	out_byte(0x1F4, (secno >> 8) & 0xFF);
	out_byte(0x1F5, (secno >> 16) & 0xFF);
	out_byte(0x1F6, 0xE0 | ((diskno&1)<<4) | ((secno>>24)&0x0F));
	out_byte(0x1F7, 0x30);	// CMD 0x30 means write sector

	for(; nsecs > 0; nsecs--, src += SECTSIZE) {
		/* block here. */
		if((r = ide_wait_ready(1)) < 0)
			return r;
		outsl(0x1F0, src, SECTSIZE/4);
	}

	return 0;
}

int write_disk(void *buf, uint32_t offset, uint32_t size)
{
	int i, op = 0, ed = 0;
	uint32_t off = offset / 512;
	uint32_t td = (uint32_t)buf - offset % 512;
	uint32_t final = (uint32_t)buf + size;

	uint8_t sect[512];
	for(; td < final; td += 512, off++)
	{
		read_section((uint32_t)sect, off);
		op = max(td, (uint32_t)buf);
		ed = min(td + 512, final);
		for(i = op; i < ed; i++)
		{
			sect[i - td] = ((uint8_t *)i)[0];
		}
		write_section(off, sect, 1);
	}
	return size;
}

void init_disk()
{
	/*
	int i, j;
	uint8_t buf[4096];
	for(i = 0; i < 7; i++)
	{
		int diff = 4096*(i+1)/7 - 4096*i/7;
		read_disk(buf, 0x2000 + 4096*i/7, diff);
		for(j = 0; j < diff; j++)
		{
			printk("%x ", buf[j]);
		}
	}
	*/
}
