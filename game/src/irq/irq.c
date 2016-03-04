#include "common.h"
#include "x86/x86.h"

void irq_handle(TrapFrame *tf)
{
	if(tf->irq > 0 && tf->irq < 1000)
	{
//		printk("0x%x\n", tf->eip);
		assert(0);
	}
	else if(tf->irq == 1000)
	{
		printk("keyboard");
	}
}
