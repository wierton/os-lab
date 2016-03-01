#ifndef __COMMON_H__
#define __COMMON_H__

#define SERIAL_PORT  0x3F8
#define BK_W 800
#define BK_H 600
#define BK_SIZE ((BK_WIDTH) * (BK_HEIGHT))

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

/* Elf header definition */
typedef struct {
	uint8_t e_ident[16];
	uint16_t e_type;
	uint16_t e_machine;
	uint32_t e_version;
	uint32_t e_entry;
	uint32_t e_phoff;
	uint32_t e_shoff;
	uint32_t e_flags;
	uint16_t e_ehsize;
	uint16_t e_phentsize;
	uint16_t e_phnum;
	uint16_t e_shentsize;
	uint16_t e_shnum;
	uint16_t e_shstrndx;
} Elf32_Endr;

/* program header definition */
typedef struct {
	uint32_t p_type;
	uint32_t p_offset;
	uint32_t p_vaddr;
	uint32_t p_paddr;
	uint32_t p_filesz;
	uint32_t p_memsz;
	uint32_t p_flags;
	uint32_t p_align;
} Elf32_Phdr;

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
