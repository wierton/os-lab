#include "common.h"
#include "font/font.h"
#include "device/video.h"

static char sztext[100] = "";

void abort(const char *file, int line)
{
	/*
	draw_rect(0, 0, SCR_W, SCR_H, make_rgb(0, 0, 0xff));
	sprintk(sztext, "Assert failed:%s, %d", file, line);
	draw_str(0, 10, 0xffffffff, 1, sztext);
	*/
	void set_bkcolor(int);
	set_bkcolor(0xff);
	printk("Assertion failed:%s, %d", file, line);
	asm volatile("cli;hlt");
}

void poweroff()
{
	draw_rect(0, 0, SCR_W, SCR_H, make_rgb(0, 0, 0));
	sprintk(sztext, "poweroff ...");
	draw_str(0, 10, 0xffffffff, 2, sztext);
	asm volatile("cli;hlt");
}
