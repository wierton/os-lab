#include "common.h"
#include "x86/x86.h"
#include "x86/memory.h"
#include "proc/proc.h"

/* since game can't access memory above 0xc0000000, this is necessary */

#define NR_SEGMENTS 10
#define SEG_KERNEL_CODE 1
#define SEG_KERNEL_DATA 2
#define SEG_USR_CODE 3
#define SEG_USR_DATA 4
#define SEG_TSS 5

#define DPL_KERNEL 0x0
#define DPL_USER 0x3

#define SEG_WRITABLE            0x2
#define SEG_READABLE            0x2
#define SEG_EXECUTABLE          0x8

static Taskstate ts;
static SegDesc gdt[NR_SEGMENTS];

static void set_segment(SegDesc *ptr, uint32_t type, uint32_t base, uint32_t lim, uint32_t dpl)
{
	ptr->limit_15_0  = lim & 0xffff;
	ptr->base_15_0   = base & 0xffff;
	ptr->base_23_16  = (base >> 16) & 0xff;
	ptr->type = type;
	ptr->segment_type = 1; //0 for app and 1 for sys
	ptr->privilege_level = dpl;
	ptr->present = 1;
	ptr->limit_19_16 = (lim >> 16) & 0xf;
	ptr->soft_use = 0;
	ptr->operation_size = 0;
	ptr->pad0 = 1;
	ptr->granularity = 1;
	ptr->base_31_24  = (base >> 24) & 0xff;
}

void init_segment()
{
	memset(gdt, 0, sizeof(gdt));

	set_segment(&gdt[SEG_KERNEL_CODE], SEG_EXECUTABLE | SEG_READABLE, 0, 0xffffffff, DPL_KERNEL);
	set_segment(&gdt[SEG_KERNEL_DATA], SEG_WRITABLE, 0, 0xffffffff, DPL_KERNEL);

	set_segment(&gdt[SEG_USR_CODE], SEG_EXECUTABLE | SEG_READABLE, 0, 0xffffffff, DPL_USER);
	set_segment(&gdt[SEG_USR_DATA], SEG_WRITABLE, 0, 0xffffffff, DPL_USER);

	/* reload gdt */
	uint16_t volatile data[3];
	data[0] = sizeof(gdt) - 1;
	data[1] = (uint32_t)gdt;
	data[2] = ((uint32_t)gdt) >> 16;
	asm volatile("lgdt (%0)"::"r"(data));
	
	/* init tss */
	extern uint32_t _stack_end_;
	uint32_t STACK_TOP = (uint32_t)(&_stack_end_);
	ts.esp0 = STACK_TOP;
	ts.ss0 = SEG_KERNEL_DATA << 3;
	set_segment(&gdt[SEG_TSS], 0x9, (uint32_t)(&ts), sizeof(ts) - 1, DPL_KERNEL);
	gdt[SEG_TSS].segment_type = 0;
	asm volatile("ltr %0"::"r"((uint16_t)(SEG_TSS << 3)));
}
