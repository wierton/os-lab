#define SERIAL_PORT  0x3F8

static inline void out_byte(short port, unsigned char data)
{
	asm volatile("out %0, %1"::"a"(data), "d"(port));
}

static inline unsigned char in_byte(short port)
{
	unsigned char data;
	asm volatile("in %1, %0":"=a"(data):"d"(port));
	return data;
}

static inline unsigned int in_long(short port)
{
	unsigned int data;
	asm volatile("in %1, %0":"=a"(data):"d"(port));
	return data;
}


static inline int serial_idle()
{
	return (in_byte(SERIAL_PORT + 5) & 0x20) != 0;
}

void __attribute__((__noinline__)) serial_printc(char ch)
{
	while (serial_idle() != 1);
	out_byte(SERIAL_PORT, ch);
}

void prints(char *str)
{
	while(*str)
	{
		serial_printc(*str++);
	}
}

int main()
{
	prints("Hello World!\n");
	while(1);
	return 0;
}
