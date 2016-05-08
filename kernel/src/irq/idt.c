#include "common.h"
#include "x86/x86.h"

#define NR_IRQ 256
#define INTERRUPT_GATE_32 0xe
#define TRAP_GATE_32 0xf

#define DPL_KERNEL              0
#define DPL_USER                3

#define NR_SEGMENTS             3
#define SEG_KERNEL_CODE         1 
#define SEG_KERNEL_DATA         2

GateDescriptor idt[NR_IRQ];

void set_trap(GateDescriptor *gdr, uint32_t offset, uint32_t selector, uint32_t dpl)
{
	gdr->offset_15_0 = offset & 0xffff;
	gdr->segment = selector << 3;
	gdr->pad0 = 0;
	gdr->type = TRAP_GATE_32;
	gdr->system = false;
	gdr->privilege_level = dpl;
	gdr->present = true;
	gdr->offset_31_16 = (offset >> 16) & 0xffff;
}

void set_intr(GateDescriptor *gdr, uint32_t offset, uint32_t selector, uint32_t dpl)
{
	gdr->offset_15_0 = offset & 0xffff;
	gdr->segment = selector << 3;
	gdr->pad0 = 0;
	gdr->type = INTERRUPT_GATE_32;
	gdr->system = false;
	gdr->privilege_level = dpl;
	gdr->present = true;
	gdr->offset_31_16 = (offset >> 16) & 0xffff;
}


void vec0();
void vec1();
void vec2();
void vec3();
void vec4();
void vec5();
void vec6();
void vec7();
void vec8();
void vec9();
void vec10();
void vec11();
void vec12();
void vec13();
void vec14();
void vecsys();

void irq0(); // timer
void irq1(); // keyboard
void irq14(); // disk

void irq_empty(); // unhandled

void init_idt()
{
	int i;
	for(i = 0; i < NR_IRQ; i++)
	{
		set_trap(idt + i, (uint32_t)irq_empty, SEG_KERNEL_CODE, DPL_KERNEL);
	}

	set_trap(idt + 0, (uint32_t)vec0, SEG_KERNEL_CODE, DPL_KERNEL);
	set_trap(idt + 1, (uint32_t)vec1, SEG_KERNEL_CODE, DPL_KERNEL);
	set_trap(idt + 2, (uint32_t)vec2, SEG_KERNEL_CODE, DPL_KERNEL);
	set_trap(idt + 3, (uint32_t)vec3, SEG_KERNEL_CODE, DPL_USER);
	set_trap(idt + 4, (uint32_t)vec4, SEG_KERNEL_CODE, DPL_USER);
	set_trap(idt + 5, (uint32_t)vec5, SEG_KERNEL_CODE, DPL_KERNEL);
	set_trap(idt + 6, (uint32_t)vec6, SEG_KERNEL_CODE, DPL_KERNEL);
	set_trap(idt + 7, (uint32_t)vec7, SEG_KERNEL_CODE, DPL_KERNEL);
	set_trap(idt + 8, (uint32_t)vec8, SEG_KERNEL_CODE, DPL_KERNEL);
	set_trap(idt + 9, (uint32_t)vec9, SEG_KERNEL_CODE, DPL_KERNEL);
	set_trap(idt + 10, (uint32_t)vec10, SEG_KERNEL_CODE, DPL_KERNEL);
	set_trap(idt + 11, (uint32_t)vec11, SEG_KERNEL_CODE, DPL_KERNEL);
	set_trap(idt + 12, (uint32_t)vec12, SEG_KERNEL_CODE, DPL_KERNEL);
	set_trap(idt + 13, (uint32_t)vec13, SEG_KERNEL_CODE, DPL_KERNEL);
	set_trap(idt + 14, (uint32_t)vec14, SEG_KERNEL_CODE, DPL_KERNEL);
	
	set_intr(idt + 0x80, (uint32_t)vecsys, SEG_KERNEL_CODE, DPL_USER);

	set_intr(idt + 32 + 0, (uint32_t)irq0, SEG_KERNEL_CODE, DPL_KERNEL);
	set_intr(idt + 32 + 1, (uint32_t)irq1, SEG_KERNEL_CODE, DPL_KERNEL);
	set_intr(idt + 32 + 14, (uint32_t)irq14, SEG_KERNEL_CODE, DPL_KERNEL);

	uint16_t volatile data[3];
	data[0] = sizeof(idt) - 1;
	data[1] = (uint32_t)idt;
	data[2] = ((uint32_t)idt) >> 16;
	asm volatile("lidt (%0)"::"r"(data));
}
