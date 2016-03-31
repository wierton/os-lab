#ifndef __X86_H__
#define __X86_H__

#include "x86/cpu.h"

typedef struct {
	uint32_t offset_15_0      : 16;
	uint32_t segment          : 16;
	uint32_t pad0             : 8;
	uint32_t type             : 4;
	uint32_t system           : 1;
	uint32_t privilege_level  : 2;
	uint32_t present          : 1;
	uint32_t offset_31_16     : 16;
} GateDescriptor;

typedef struct {
	/* pusha */
	uint32_t edi, esi, ebp, old_esp, ebx, edx, ecx, eax;
	/**/
	uint16_t es, pad0;
	uint16_t ds, pad1;
	int32_t irq;
	/* defined by hardware */
	uint32_t err;
	uint32_t eip;
	uint16_t cs, pad2;
	uint32_t eflags;
	/* cross rings */
	uint32_t esp;
	uint16_t ss, pad3;
} TrapFrame;

#endif
