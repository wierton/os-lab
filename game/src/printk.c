#include "common.h"

void init_serial()
{
	out_byte(SERIAL_PORT + 1, 0x00);
	out_byte(SERIAL_PORT + 3, 0x80);
	out_byte(SERIAL_PORT + 0, 0x01);
	out_byte(SERIAL_PORT + 1, 0x00);
	out_byte(SERIAL_PORT + 3, 0x03);
	out_byte(SERIAL_PORT + 2, 0xC7);
	out_byte(SERIAL_PORT + 4, 0x0B);
}

static inline int serial_idle()
{
	return (in_byte(SERIAL_PORT + 5) & 0x20) != 0;
}

void serial_printc(char ch)
{
	while (serial_idle() != 1);
	out_byte(SERIAL_PORT, ch);
}

inline void prints(char *str)
{
	if(str != NULL)
		for(;*str != '\0'; str ++) serial_printc(*str);
}

void printd(int val)
{
	int i = 15;
	bool IsNeg = false;
	char buf[20] = {0};
	if(val < 0)
	{
		IsNeg = true;
	}
	do
	{
		int temp = val % 10;
		temp = temp < 0 ? (-temp) : temp;
		buf[i--] = '0' + temp;
		val /= 10;
	} while(val);
	
	if(IsNeg)
		buf[i--] = '-';

	prints(&buf[i + 1]);
}

void printb(uint8_t val)
{
	int op = (val >> 4) & 0xf, ed = val & 0xf;
	if(op < 0xa)
		serial_printc(op + '0');
	else
		serial_printc(op - 0xa + 'a');
	
	if(ed < 0xa)
		serial_printc(ed + '0');
	else
		serial_printc(ed - 0xa + 'a');
}

void printx(uint32_t val)
{
	int i, pos = 0;
	char buf[20];
	bool IsPrefix = true;
	for(i = 7; i >= 0; i --)
	{
		uint8_t temp = (val >> (4 * i)) & 0xf;
		if(temp == 0 && IsPrefix)
			continue;
		IsPrefix = false;
		if(temp < 0xa)
			buf[pos ++] = '0' + temp;
		else
			buf[pos ++] = 'a' + temp - 0xa;
	}
	if(pos == 0)
		buf[pos ++] = '0';
	buf[pos] = 0;
	prints((void *)buf);
}

void __attribute__((noinline)) printk(const char *ctl, ...) {
	void **args = ((void **)(&ctl)) + 1;
	int i = 0, pargs = 0;
	for(;ctl[i] != '\0'; i ++)
	{
		if(ctl[i] != '%')
		{
			serial_printc(ctl[i]);
			continue;
		}
		switch(ctl[++ i])
		{
			case 'c':
			case 'C':
				serial_printc((char)(((int *)args)[pargs ++]));
				break;
			case 'd':
			case 'D':
				printd((((int *)args)[pargs ++]));
				break;
			case 's':
			case 'S':
				prints((void *)(((int *)args)[pargs ++]));
				break;
			case 'x':
			case 'X':
				printx((((uint32_t *)args)[pargs ++]));
				break;
			case 'b':
			case 'B':
				printb((((uint8_t *)args)[pargs ++]));
				break;
			default:
				break;
		}
	}
}

void test_printk()
{
	printk("Printk test begin...\n");
	printk("the answer should be:\n");
	printk("#######################################################\n");
	printk("Hello, welcome to OSlab! I'm the body of the game. ");
	printk("Bootblock loads me to the memory position of 0x100000, and Makefile also tells me that I'm at the location of 0x100000. ");
	printk("~!@#$^&*()_+`1234567890-=...... ");
	printk("Now I will test your printk: ");
	printk("1 + 1 = 2, 123 * 456 = 56088\n0, -1, -2147483648, -1412505855, -32768, 102030\n0, ffffffff, 80000000, abcdef01, ffff8000, 18e8e\n");
	printk("#######################################################\n");
	printk("your answer:\n");
	printk("=======================================================\n");
	printk("%s %s%scome %co%s", "Hello,", "", "wel", 't', " ");
	printk("%c%c%c%c%c! ", 'O', 'S', 'l', 'a', 'b');
	printk("I'm the %s of %s. %s 0x%x, %s 0x%x. ", "body", "the game", "Bootblock loads me to the memory position of",
    0x100000, "and Makefile also tells me that I'm at the location of", 0x100000);
	printk("~!@#$^&*()_+`1234567890-=...... ");
	printk("Now I will test your printk: ");
	printk("%d + %d = %d, %d * %d = %d\n", 1, 1, 1 + 1, 123, 456, 123 * 456);
	printk("%d, %d, %d, %d, %d, %d\n", 0, 0xffffffff, 0x80000000, 0xabcedf01, -32768, 102030);
	printk("%x, %x, %x, %x, %x, %x\n", 0, 0xffffffff, 0x80000000, 0xabcedf01, -32768, 102030);
	printk("=======================================================\n");
	printk("Test end!!! Good luck!!!\n");
}
