#include "common.h"
#include "font/font.h"
#include "device/video.h"

static char sztext[100] = "";

void abort(const char *file, int line)
{
	draw_rect(0, 0, BK_W, BK_H, make_rgb(0, 0, 0xff));
	sprintk(sztext, "Assert failed:%s, %d", file, line);
	draw_str(0, 10, 0, 1, sztext);
	asm volatile("cli;hlt");
}
