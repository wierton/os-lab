#include "common.h"
#include "x86/x86.h"

static char *dststr = NULL;
typedef void (*PRINTER)(char);

void init_serial()
{
#ifdef VIRTUAL
	out_byte(SERIAL_PORT + 1, 0x00);
	out_byte(SERIAL_PORT + 3, 0x80);
	out_byte(SERIAL_PORT + 0, 0x01);
	out_byte(SERIAL_PORT + 1, 0x00);
	out_byte(SERIAL_PORT + 3, 0x03);
	out_byte(SERIAL_PORT + 2, 0xC7);
	out_byte(SERIAL_PORT + 4, 0x0B);
#endif
}

static inline int serial_idle()
{
	return (in_byte(SERIAL_PORT + 5) & 0x20) != 0;
}

void serial_printc(char ch)
{
#ifdef VIRTUAL
	while (serial_idle() != 1);
	out_byte(SERIAL_PORT, ch);
#endif
}

void sprintc(char ch)
{
	if(dststr != NULL)
		*dststr++ = ch;
}

inline void prints(char *str, PRINTER printer)
{
	if(str != NULL)
		for(;*str != '\0'; str ++) printer(*str);
}

void printd(int val, PRINTER printer)
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

	prints(&buf[i + 1], printer);
}

void printx(uint32_t val, PRINTER printer)
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
	prints((void *)buf, printer);
}

int __attribute__((noinline)) vfprintf(const char *ctl, void **args, PRINTER printer) {
	int i = 0, pargs = 0;
	for(;ctl[i] != '\0'; i ++)
	{
		if(ctl[i] != '%')
		{
			printer(ctl[i]);
			continue;
		}
		switch(ctl[++ i])
		{
			case 'c':
			case 'C':
				printer((char)(((int *)args)[pargs ++]));
				break;
			case 'd':
			case 'D':
				printd((((int *)args)[pargs ++]), printer);
				break;
			case 's':
			case 'S':
				prints((void *)(((int *)args)[pargs ++]), printer);
				break;
			case 'p':
			case 'P':
			case 'x':
			case 'X':
				printx((((uint32_t *)args)[pargs ++]), printer);
				break;
			default:
				break;
		}
	}
	return 0;
}

void __attribute__((noinline)) printk(const char *ctl, ...)
{
	void **args = ((void **)(&ctl)) + 1;
	vfprintf(ctl, args, serial_printc);
}

void __attribute__((noinline)) sprintk(char *dst, const char *ctl, ...)
{
	void **args = ((void **)(&ctl)) + 1;
	dststr = dst;
	vfprintf(ctl, args, sprintc);
	sprintc('\0');
	dststr = NULL;
}

void __attribute__((noinline)) vsprintk(char *dst, const char *ctl, void **args)
{
	dststr = dst;
	vfprintf(ctl, args, sprintc);
	sprintc('\0');
	dststr = NULL;
}

int sys_printc(TrapFrame *tf)
{
	serial_printc(tf->ebx);
	return 0;
}

int sys_prints(TrapFrame *tf)
{
	int i;
	for(i = 0; i < tf->edx; i++)
	{
		serial_printc(((char *)(i + tf->ecx))[0]);
	}
	return 0;
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
