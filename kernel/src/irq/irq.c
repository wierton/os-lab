#include "common.h"
#include "x86/x86.h"

void timer_event();
void keyboard_event(uint32_t);
void do_syscall(TrapFrame *tf);
void switch_thread(TrapFrame *tf);

void irq_handle(TrapFrame *tf)
{
	if(tf->irq < 1000)
	{
		if(tf->irq == 0x80)
		{
			do_syscall(tf);
		}
		else
		{
			printk("\033[1;31mUnhandled exception : #%d at 0x%x\n\033[0m", tf->irq, tf->eip);
			assert(0);
		}
	}
	else if(tf->irq == 1000)
	{
		timer_event();
		switch_thread(tf);
	}
	else if(tf->irq == 1001)
	{
		uint32_t code = in_byte(0x60);
		uint32_t val = in_byte(0x61);
		out_byte(0x61, val | 0x80);
		out_byte(0x61, val);
		keyboard_event(code);
	}
	else if(tf->irq == 1014)
	{}
	else
	{
		assert(0);
	}
}
