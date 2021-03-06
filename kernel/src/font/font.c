#include "common.h"
#include "font/font.h"
#include "font/basic-font.h"
#include "device/video.h"
#include "string.h"

#define CHAR_W 8
#define CHAR_H 8

static int bkcolor = 0x00000000;

void set_bkcolor(int color)
{
	if(color != bkcolor)
	{
		int i, j;
		int bpl = ((3 * SCR_W + 3) & ~3);
		for(i = 0; i < SCR_W; i++)
			for(j = 0; j < SCR_H; j++)
			{
				int pos = VMEM + 3 * i + j * bpl;
				uint32_t *vmem = (uint32_t *)(pos);
				if((vmem[0] & 0xffffff) == (bkcolor & 0xffffff))
					vmem[0] = color;
			}
	}
	bkcolor = color;
}

void draw_character(char ch, int x, int y, uint32_t color, uint8_t times)
{
	uint32_t i, j;
	
	for(i = 0; i < CHAR_W * times; i++)
		for(j = 0; j < CHAR_H * times; j++)
		{
			uint32_t tar = 3 * (x + i) + (y + j) * (3 * SCR_W);
			uint32_t *vmem = (uint32_t *)(VMEM + tar);
			if((font8x8_basic[(uint8_t)ch][j / times] >> (i / times)) & 0x1)
			{
				vmem[0] = color;
			}
			else
				vmem[0] = bkcolor;
		}

}

void draw_str(int x, int y, uint32_t color, uint8_t times, const char *ctl, ...)
{
	char str[50];
	sprintk(str, ctl);
	int i, len = strlen(str);
	for(i = 0; i < len; i++)
	{
		draw_character(str[i], x + i * times * CHAR_H, y, color, times);
	}
}

void init_font()
{
	draw_str(128, 224, make_rgb(0x80, 0x10, 0x50), 4, "Hello World!");
}
