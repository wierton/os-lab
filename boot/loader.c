#include "common.h"

#define DEBUG
#ifdef DEBUG

static char *dststr = NULL;
typedef void (*PRINTER)(char);

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

void printk(const char *ctl, ...)
{
	void **args = ((void **)(&ctl)) + 1;
	vfprintf(ctl, args, serial_printc);
}

void sprintk(char *dst, const char *ctl, ...)
{
	void **args = ((void **)(&ctl)) + 1;
	dststr = dst;
	vfprintf(ctl, args, sprintc);
	sprintc('\0');
	dststr = NULL;
}
#endif

/*端口号     读还是写    具体含义
 * 1F0H       读/写      用来传送读/写的数据(其内容是正在传输的一个字节的数据)
 * 1F1H       读         用来读取错误码
 * 1F2H       读/写      用来放入要读写的扇区数量
 * 1F3H       读/写      用来放入要读写的扇区号码
 * 1F4H       读/写      用来存放读写柱面的低8位字节
 * 1F5H       读/写      用来存放读写柱面的高2位字节(其高6位恒为0)
 * 1F6H       读/写      用来存放要读/写的磁盘号及磁头号
 *	--第7位     恒为1
 *	--第6位     恒为0
 *	--第5位     恒为1
 *	--第4位     为0代表第一块硬盘、为1代表第二块硬盘
 *	--第3~0位   用来存放要读/写的磁头号
 * 1f7H
 * -读          用来存放读操作后的状态
 *	--第7位     控制器忙碌
 *	--第6位     磁盘驱动器准备好了
 *	--第5位     写入错误
 *	--第4位     搜索完成
 *	--第3位     为1时扇区缓冲区没有准备好
 *	--第2位     是否正确读取磁盘数据
 *	--第1位     磁盘每转一周将此位设为1,
 *	--第0位     之前的命令因发生错误而结束
 *----------------------------------------------------------------
 *	-写         该位端口为命令端口,用来发出指定命令
 *	--为50h     格式化磁道
 *	--为20h     尝试读取扇区
 *	--为21h     无须验证扇区是否准备好而直接读扇区
 *	--为22h     尝试读取长扇区(用于早期的硬盘,每扇可能不是512字节,而是128字节到1024之间的值)
 *	--为23h     无须验证扇区是否准备好而直接读长扇区
 *	--为30h     尝试写扇区
 *	--为31h     无须验证扇区是否准备好而直接写扇区
 *	--为32h     尝试写长扇区
 *	--为33h     无须验证扇区是否准备好而直接写长扇区
 */

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

void loader()
{
	int i, j;
	Elf32_Endr *elf = (Elf32_Endr *)0xA000;
	Elf32_Phdr *ph;
	read_disk((uint32_t)elf, 0, 0x1000);

	ph = (void *)elf + elf->e_phoff;

	for(i = 0; i < elf->e_phnum; i++)
	{
		read_disk(ph->p_paddr, ph->p_offset, ph->p_filesz);
		/*zero the memory [paddr + filesz, paddr + memsz)*/
		for(j = ph->p_paddr + ph->p_filesz; j < ph->p_paddr + ph->p_memsz; j++)
		{
			((uint8_t *)j)[0] = 0;
		}
		ph++;
	}

	asm volatile("push %0;ret"::"a"(elf->e_entry));
}
