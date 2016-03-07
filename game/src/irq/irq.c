#include "common.h"
#include "x86/x86.h"

void (*do_timer)(void);
void (*do_keyboard)(uint32_t);

void set_timer(void *doer)
{
	do_timer = doer;
}

void set_keyboard(void *doer)
{
	do_keyboard = doer;
}

void irq_handle(TrapFrame *tf)
{
	if(tf->irq < 1000)
	{
		printk("Unhandled exception : #%d at 0x%x\n", tf->irq, tf->eip);
		assert(0);
	}
	else if(tf->irq == 1000)
	{
		do_timer();
	}
	else if(tf->irq == 1001)
	{
		uint32_t code = in_byte(0x60);
		uint32_t val = in_byte(0x61);
		out_byte(0x61, val | 0x80);
		out_byte(0x61, val);
		do_keyboard(code);
	}
	else
	{
		assert(0);
	}
}
