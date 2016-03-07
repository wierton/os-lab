#include "common.h"
#include "x86/x86.h"

void init_8259();
void init_timer();
void init_idt();
void init_keyboard();
void init_serial();
void init_video();
void init_disk();
void init_font();
void test_printk();
void set_timer(void *doer);
void set_keyboard(void *doer);
void keyboard_event(uint32_t code);
void timer_event(void);
void game_init();

int main()
{
	init_video();
	init_serial();
	init_timer();
	init_idt();
	init_keyboard();
	init_8259();
	set_timer(timer_event);
	set_keyboard(keyboard_event);
	init_disk();
	init_font();
	test_printk();
	sti();
	game_init();
	while(1);
	return 0;
}
