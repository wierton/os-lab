#ifndef __COMMON_H__
#define __COMMON_H__

#include "type.h"
#include "string.h"

#define VIRTUAL
#define SERIAL_PORT  0x3F8

#define KERNEL_BASE 0xc0000000
#define KERNEL_PHYBASE 0x100000
#define KERNEL_SIZE (1 << 17)
#define va_to_pa(addr) (((void *)(addr)) - KERNEL_BASE)
#define pa_to_va(addr) (((void *)(addr)) + KERNEL_BASE)

void printk(const char *ctl, ...);
void sprintk(char *dst, const char *ctl, ...);
void vsprintk(char *dst, const char *ctl, void **args);

static inline void out_byte(short port, uint8_t data)
{
	asm volatile("out %0, %1"::"a"(data), "d"(port));
}

static inline void out_word(short port, uint16_t data)
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

void abort(const char *file, int line);

#define assert(cond)\
	do { \
		if(!(cond)) \
		{ \
			abort(__FILE__, __LINE__); \
		} \
	} while(0)

#endif
