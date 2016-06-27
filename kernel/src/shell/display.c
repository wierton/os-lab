#include "common.h"
#include "shell/comarg.h"
#include "font/font.h"
#include "device/time.h"
#include "x86/x86.h"

int back_st;
int caretx = 0, carety = 0;
char cbuf[CHAR_R][CHAR_L];

int set_backst(int val)
{
	back_st = val;
	return 0;
}

void overload()
{
	int i, j;
	for(j = 0; j < CHAR_L - 1; j++)
		for(i = 0; i < CHAR_R; i++)
		{
			cbuf[i][j] = cbuf[i][j + 1];
			draw_character(' ', i * D_CHAR_W, j * D_CHAR_W, 0xffffffff, TIMES);
			draw_character(cbuf[i][j], i * D_CHAR_W, j * D_CHAR_W, 0xffffffff, TIMES);
		}
	for(i = 0; i < CHAR_R; i++)
	{
		cbuf[i][CHAR_L - 1] = '\0';
		draw_character(' ', i * D_CHAR_W, (CHAR_L - 1) * D_CHAR_W, 0xffffffff, TIMES);
	}
}

void write_char(char ch)
{
	static int cx = 0, cy = 0;
	draw_character(cbuf[cx][cy], cx * D_CHAR_W, cy * D_CHAR_W, 0xffffffff, TIMES);
	switch(ch)
	{
		case '\0':return;
		case '\n':
				  cx = caretx; cy = carety;
				  caretx = 0;
				  carety ++;
				  if(carety >= CHAR_L)
				  {
					  overload();
					  carety = CHAR_L - 1;
				  }
				  return;
		case '\r':caretx = 0;return;
		case '\t':caretx = (caretx + 7) & (~7);return;
		case '\b':
				  if(caretx > back_st)
				  {
					  caretx --;
					  cbuf[caretx][carety] = '\0';
					  draw_character(' ', (caretx + 1) * D_CHAR_W, carety * D_CHAR_W, 0xffffffff, TIMES);
				  }
				  return;
	}

	if(caretx >= CHAR_R)
	{
		caretx = 0;
		carety ++;
		if(carety < CHAR_L)
		{
			cbuf[caretx][carety] = ch;
			draw_character(ch, caretx * D_CHAR_W, carety * D_CHAR_W, 0xffffffff, TIMES);
			cx = caretx; cy = carety;
			caretx ++;
		}
		else
		{
			overload();
			carety = CHAR_L - 1;
			cbuf[caretx][carety] = ch;
			draw_character(ch, caretx * D_CHAR_W, carety * D_CHAR_W, 0xffffffff, TIMES);
			cx = caretx; cy = carety;
			caretx ++;
		}
	}
	else
	{
		cbuf[caretx][carety] = ch;
		draw_character(ch, caretx * D_CHAR_W, carety * D_CHAR_W, 0xffffffff, TIMES);
		cx = caretx; cy = carety;
		caretx ++;
	}
}

void show_caret()
{
	int cx = caretx, cy = carety;
	static int up = 0, ti = 0;
	if(time() - ti > 50)
	{
		if(cx >= CHAR_R)
		{
			cx = 0;
			cy ++;
		}
		if(cy >= CHAR_L)
		{
			write_char(' ');
			write_char('\r');
			cy = CHAR_L - 1;
		}
		ti = time();
		up = !up;
		if(up)
			draw_character(1, cx * D_CHAR_W, cy * D_CHAR_W, 0xffffffff, TIMES);
		else
			draw_character(' ', cx * D_CHAR_W, cy * D_CHAR_W, 0xffffffff, TIMES);
	}
}

void init_console()
{
	int i, j;
	for(j = 0; j < CHAR_L; j++)
		for(i = 0; i < CHAR_R; i++)
		{
			cbuf[i][j] = 0;
			draw_character('\0', i * D_CHAR_W, j * D_CHAR_W, 0xffffffff, TIMES);
		}
	printk("Init Console ...\n");
	set_backst(2);
	extern int forbid_switch;
	forbid_switch = 1;
}

int shell(TrapFrame *tf)
{
	extern int forbid_switch, user_mode;
	asm volatile("sti");
	user_mode = 0;
	forbid_switch = 1;
	printk("/>");
	set_backst(2);
	while(1)
	{
		show_caret();
	}
}
