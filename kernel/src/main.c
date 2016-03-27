#include "common.h"
#include "x86/x86.h"

void init_page();
void init_mm();
void init_udir();
void init_8259();
void init_timer();
void init_idt();
void init_keyboard();
void init_serial();
void init_video();
void init_disk();
void init_font();
void test_printk();
void init_segment();
void load_game();

void init_cond();

extern uint32_t _stack_end_;

void init_prev()
{
	init_page();
	asm volatile("movl %0, %%esp"::"a"(&_stack_end_));
	asm volatile("jmpl *%0"::"r"(init_cond));
}

void init_cond()
{
	init_segment();

	init_idt();
	init_serial();
	init_8259();
	init_timer();
	init_keyboard();
	init_video();
	sti();

	init_udir();
	init_mm();
	init_font();
	load_game();
	while(1);
}
