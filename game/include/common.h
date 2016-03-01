#ifndef __COMMON_H__
#define __COMMON_H__

#include "lib.h"

#define SERIAL_PORT  0x3F8

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

#endif
