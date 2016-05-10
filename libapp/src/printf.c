#include "common.h"
#include "syscall.h"
#include "lib.h"

static int buflen = 0;
static char *dststr = NULL, buf[1000];

inline void sprintc(char ch)
{
	if(dststr != NULL)
		*dststr++ = ch;
}

inline int prints(char *str)
{
	char *tmp = str;
	if(str != NULL)
		for(;*str != '\0'; str ++)
			sprintc(*str);
	return (int)(str - tmp);
}

int printd(int val)
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
	return 15 - i;
}

int printx(uint32_t val)
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
	return pos;
}

int vfprintf(const char *ctl, void **args) {
	int i = 0, pargs = 0;
	for(;ctl[i] != '\0'; i ++)
	{
		if(ctl[i] != '%')
		{
			sprintc(ctl[i]);
			buflen ++;
			continue;
		}
		switch(ctl[++ i])
		{
			case 'c':
			case 'C':
				buflen ++;
			   	sprintc((char)(((int *)args)[pargs ++]));
				break;
			case 'd':
			case 'D':
				buflen += printd((((int *)args)[pargs ++]));
				break;
			case 's':
			case 'S':
				buflen += prints((void *)(((int *)args)[pargs ++]));
				break;
			case 'p':
			case 'P':
			case 'x':
			case 'X':
				buflen += printx((((uint32_t *)args)[pargs ++]));
				break;
			default:
				break;
		}
	}
	return 0;
}

void vsprintf(char *dst, const char *ctl, void **args)
{
	dststr = dst;
	vfprintf(ctl, args);
	sprintc('\0');
	dststr = NULL;
}

void __attribute__((noinline)) printf(const char *ctl, ...)
{
	void **args = ((void **)(&ctl)) + 1;
	dststr = buf;
	buflen = 0;
	vfprintf(ctl, args);
	sprintc('\0');
	syscall(9, 2, buf, buflen);
	dststr = NULL;
}

void __attribute__((noinline)) sprintf(char *dst, const char *ctl, ...)
{
	void **args = ((void **)(&ctl)) + 1;
	buflen = 0;
	dststr = dst;
	vfprintf(ctl, args);
	sprintc('\0');
	dststr = NULL;
}

void test_printf()
{
	printf("printf test begin...\n");
	printf("the answer should be:\n");
	printf("#######################################################\n");
	printf("Hello, welcome to OSlab! I'm the body of the game. ");
	printf("Bootblock loads me to the memory position of 0x100000, and Makefile also tells me that I'm at the location of 0x100000. ");
	printf("~!@#$^&*()_+`1234567890-=...... ");
	printf("Now I will test your printf: ");
	printf("1 + 1 = 2, 123 * 456 = 56088\n0, -1, -2147483648, -1412505855, -32768, 102030\n0, ffffffff, 80000000, abcdef01, ffff8000, 18e8e\n");
	printf("#######################################################\n");
	printf("your answer:\n");
	printf("=======================================================\n");
	printf("%s %s%scome %co%s", "Hello,", "", "wel", 't', " ");
	printf("%c%c%c%c%c! ", 'O', 'S', 'l', 'a', 'b');
	printf("I'm the %s of %s. %s 0x%x, %s 0x%x. ", "body", "the game", "Bootblock loads me to the memory position of",
    0x100000, "and Makefile also tells me that I'm at the location of", 0x100000);
	printf("~!@#$^&*()_+`1234567890-=...... ");
	printf("Now I will test your printf: ");
	printf("%d + %d = %d, %d * %d = %d\n", 1, 1, 1 + 1, 123, 456, 123 * 456);
	printf("%d, %d, %d, %d, %d, %d\n", 0, 0xffffffff, 0x80000000, 0xabcedf01, -32768, 102030);
	printf("%x, %x, %x, %x, %x, %x\n", 0, 0xffffffff, 0x80000000, 0xabcedf01, -32768, 102030);
	printf("=======================================================\n");
	printf("Test end!!! Good luck!!!\n");
}
