#ifndef __X86_H__
#define __X86_H__

#include "x86/cpu.h"

#define NR_IRQ 256
#define INTERRUPT_GATE_32 0xe
#define TRAP_GATE_32 0xf

#define DPL_KERNEL              0
#define DPL_USER                3

#define NR_SEGMENTS             3
#define SEG_KERNEL_CODE         1 
#define SEG_KERNEL_DATA         2

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
	uint32_t edi, esi, ebp, old_esp, ebx, edx, ecx, eax;
	int32_t irq;
	uint32_t eip, cs, eflags;
} TrapFrame;


#endif
