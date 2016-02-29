#define SERIAL_PORT  0x3F8
#define BK_W 320
#define BK_H 200
#define BK_SIZE ((BK_WIDTH) * (BK_HEIGHT))
#define VMEM  ((uint8_t*)0xA0000)

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

#define NULL ((void *) 0)
typedef int bool;
typedef char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
#define true 1
#define false 0

static inline void out_byte(short port, uint8_t data)
{
	asm volatile("out %0, %1"::"a"(data), "d"(port));
}

static inline uint8_t in_byte(short port)
{
	uint8_t data;
	asm volatile("in %1, %0":"=a"(data):"d"(port));
	return data;
}

static inline uint32_t in_long(short port)
{
	uint32_t data;
	asm volatile("in %1, %0":"=a"(data):"d"(port));
	return data;
}

#define DEBUG

#ifdef DEBUG

static inline void init_serial()
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

static inline void prints(char *str)
{
//	if(str != NULL)
		for(;*str != '\0'; str ++) serial_printc(*str);
}
/*
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
*/


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
	serial_printc(' ');
}

#endif
#ifdef DEBUG_DRAW

static inline void draw_rect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint8_t color)
{
	/*clip the area firstly to make it safe to video memory
	 */
	if(x >= BK_W || y >= BK_H || (x + w) < 0 || (y + h) < 0)
		return;
	/*caculate the intersection area of two rectangle
	 */
	int16_t opx = max(0, x);
	int16_t opy = max(0, y);
	int16_t edx = min((int)BK_W, x + w);
	int16_t edy = min((int)BK_H, y + h);
	int16_t nw = edx - opx;
	int i, j;
	uint8_t *vmem = VMEM + opx + opy * BK_W;
	for(j = opy; j < edy; j++)
	{
		for(i = opx; i < edx; i++)
		{
			vmem[0] = color;
			vmem ++;
		}
		vmem = vmem - nw + BK_W;
	}
}

#endif

/*code above aim to test some API
 */

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
	//the byte read from 0x1f7 port with 01b in high two bits indicate the disk is prepared
	while((in_byte(0x1f7) & 0x80) != 0x0);
}

/*
inline void read_disk(uint32_t *dst, uint32_t offset, uint32_t size)
{
	wait_disk();
}
*/

void read_sect()
{
	wait_disk();
	out_byte(0x1f2, 1);
	out_byte(0x1f3, 1);
	out_byte(0x1f4, 1 >> 8);
	out_byte(0x1f5, 1 >> 16);
	out_byte(0x1f6, (1 >> 18) | 0xa0);
	out_byte(0x1f7, 0x20);
	wait_disk();
	uint32_t *dst = (uint32_t *)0x10000, *tar = dst + 512 / 4;
	for(;dst < tar;dst++)
	{
		*dst = in_long(0x1f0);
		printx(*dst);
	}
	printx(in_byte(0x1f7));
}

void loader()
{
	read_sect();
	while(1);
}
