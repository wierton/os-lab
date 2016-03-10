#include "body/game-common.h"

extern SURFACE * screen;

uint32_t read_key();

int read_menu()
{
	const uint32_t times = 3;

	int redraw = 1, sel = 0x0, count = 0;
	uint32_t color[] = {0x3f, 0x3f3f};
	uint32_t sel_color = 0x0;

	while(true)
	{
		uint32_t key = read_key();
		uint32_t keystate = (key >> 7 ) & 1;
		uint32_t keycode = key & 0xff7f;

		if(redraw)
		{
			redraw = 0;
			update_screen();
			font_out((BK_W - 11 * times * 8) / 2, (BK_H - 2 * times * 8) / 2,
				0x3f, times, "random mode");
			font_out((BK_W - 9 * times * 8) / 2, (BK_H + 2 * times * 8) / 2,
				0x3f, times, "plot mode");

		}

		if(keystate == OS_KEYDOWN)
		{
			switch(keycode)
			{
				case KEY_UP:
				case KEY_DOWN:
				case KEY_LEFT:
				case KEY_RIGHT:
					sel ^= 0x1;
					if(sel & 0x1)
					{
						font_out((BK_W - 11 * times * 8) / 2, (BK_H - 2 * times * 8) / 2, color[0], times, "random mode");
					}
					else
					{
						font_out((BK_W - 9 * times * 8) / 2, (BK_H + 2 * times * 8) / 2, color[0], times, "plot mode");
					}
					break;
				case KEY_RETURN:
					return sel & 1;
				default:break;
			}
		}

		switch(sel & 0x1)
		{
			case 0:
				font_out((BK_W - 11 * times * 8) / 2, (BK_H - 2 * times * 8) / 2,
			color[sel_color], times, "random mode");
				break;
			case 1:
				font_out((BK_W - 9 * times * 8) / 2, (BK_H + 2 * times * 8) / 2,
			color[sel_color], times, "plot mode");
				break;
			default:break;
		}
		
		if(((count ++) & 0x3f) == 0)
			sel_color ^= 0x1;


		flip(screen);
	}
	return 0xffffffff;
}
