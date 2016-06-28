#include "common.h"
#include "shell/comarg.h"
#include "font/font.h"
#include "device/time.h"
#include "x86/x86.h"

int back_st;
int caretx = 0, carety = 0;
static int color = 0xffffffff;
extern uint32_t VMEM;

void block_curt();

int set_backst(int val)
{
	back_st = val;
	return 0;
}

void overload()
{
	int i, j;
	int bpl = (3 * SCR_W + 3) & ~3;
	for(j = 0; j < SCR_H - D_CHAR_H; j++)
		for(i = 0; i < SCR_W; i++)
		{
			uint32_t *dv = (uint32_t *)(VMEM + 3 * i + j * bpl);
			uint32_t *sv = (uint32_t *)(VMEM + 3 * i + (j + D_CHAR_H) * bpl);
			dv[0] = sv[0];
			
		}
	for(i = 0; i < CHAR_R; i++)
	{
		draw_character(' ', i * D_CHAR_W, (CHAR_L - 1) * D_CHAR_W, 0xffffffff, TIMES);
	}
}

int switch_mode(char ch)
{
	static int transfer = 0;

	switch(ch)
	{
		case '1':if(transfer == 5){transfer = 11;return 1;}break;
		case '2':if(transfer == 5){transfer = 12;return 1;}break;
		case '3':if(transfer == 5){transfer = 13;return 1;}break;
		case '4':if(transfer == 5){transfer = 14;return 1;}break;
		case '5':if(transfer == 5){transfer = 15;return 1;}break;
		case '6':if(transfer == 5){transfer = 16;return 1;}break;
		case '7':if(transfer == 5){transfer = 17;return 1;}break;
		case '8':if(transfer == 5){transfer = 18;return 1;}break;
		case '9':if(transfer == 5){transfer = 19;return 1;}break;
		case 'm':if(transfer > 10)
				 {
					 switch(transfer)
					 {
						 case 11:color = 0x00ff0000;break;
						 case 12:color = 0x0000ff00;break;
						 case 13:color = 0x00ffff00;break;
						 case 14:color = 0x000000ff;break;
						 case 15:color = 0x00aa00ff;break;
						 case 16:color = 0x0000aaff;break;
						 case 17:color = 0x00ffffff;break;
						 default:color = 0x00ffffff;break;
					 }
					 transfer = 0;
					 return 1;
				 }
				 break;
	}

	switch(ch)
	{
		case '\033':transfer = 1;return 1;
		case '[':if(transfer == 1){transfer = 2;return 1;}break;
		case '1':if(transfer == 2){transfer = 3;return 1;}break;
		case ';':if(transfer == 3){transfer = 4;return 1;}break;
		case '3':if(transfer == 4){transfer = 5;return 1;}break;
		case '0':if(transfer == 2){transfer = 8;return 1;}break;
		case 'm':if(transfer == 8)
				 {
					 color = 0xffffffff;
					 transfer = 0;
					 return 1;
				 }
		default :transfer = 0;return 0;
	}
	return 0;
}

void write_char(char ch)
{
	/* for debug */
	/*
	void serial_printc(char);
	serial_printc(ch);
	return;
	*/

	//draw_character(old_ch, cx * D_CHAR_W, cy * D_CHAR_W, color, TIMES);
	if(switch_mode(ch))
		return;
	switch(ch)
	{
		case '\0':return;
		case '\n':
				  draw_character(' ', caretx * D_CHAR_W, carety * D_CHAR_W, 0xffffffff, TIMES);
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
			draw_character(ch, caretx * D_CHAR_W, carety * D_CHAR_W, color, TIMES);
			caretx ++;
		}
		else
		{
			overload();
			carety = CHAR_L - 1;
			draw_character(ch, caretx * D_CHAR_W, carety * D_CHAR_W, color, TIMES);
			caretx ++;
		}
	}
	else
	{
		draw_character(ch, caretx * D_CHAR_W, carety * D_CHAR_W, color, TIMES);
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
			draw_character('\0', i * D_CHAR_W, j * D_CHAR_W, 0xffffffff, TIMES);
		}
	printk("Init Console ...\n");
	set_backst(2);
	extern int forbid_switch;
	forbid_switch = 1;
}

int shell(TrapFrame *tf)
{
	block_curt();
	extern int forbid_switch, user_mode;
	user_mode = 0;
	forbid_switch = 1;
	asm volatile("sti");
	printk("/>");
	set_backst(2);
	while(1)
	{
		show_caret();
	}
}
