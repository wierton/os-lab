#include "body/game-common.h"
#include "body/bullet.h"
#include "body/plane.h"
#include "body/bomb.h"

#define NR_ENEMY 40

/* handle pool
 * 小心封装过度
 * game的功能
 * 移动所有子弹
 * 移动所有飞机（player由按键事件移动），并绘制所有飞机
 * 解析剧情
 */

typedef enum {GAME_START, GAME_PROCESSING, GAME_END, GAME_RESUME, GAME_UNKNOWN} GAME_STATE;

static GAME_STATE game_state = GAME_UNKNOWN;

void add_drop_xbullets(char ch);
void rand_drop_xbullets();
void move_drop_xbullets();
void collision_drop_xbullets();
void show_player();
void resume_player();
void draw_toolbar();
void delay(uint32_t ms);

extern SURFACE *screen;

int count = 0;

void game_main()
{
	int i;
	const int NR_COUNT = 40;
	HANDLE handle[NR_COUNT];
	
	resume_player();
	
	IBS ibs = {ID_DOT, ID_ENEMY, IDBF_SPEEDLINE, PI / 3 + 0.25, 2 * PI / 3, 1};
	
	for(i = 0; i < NR_COUNT; i++)
	{
		handle[i] = alloc_bullet(35);
		set_bullet(handle[i], &ibs);
	}
/*
	PLANE_HELPER plane_helper = {make_point(100, 80), 1, 0, 0};

	i = 0;
*/	while(game_state != GAME_END)
	{
/*		i++;
		judge_collision();
		if(i % 100 == 0)
		{
			LPPLANE newplane = alloc_plane();
			plane_helper.pos.x = rand() % (BK_W - 100) + 50;
			plane_helper.pos.y = 1;
			set_plane(newplane, ID_ENEMY, IDPF_LINE, &plane_helper, 3, &ibs);
		}
		if(i % 2048 == 0)
		{
			rand_drop_xbullets();
		}
*/
		msgloop();
		update_screen();
/*
		move_plane();
		show_plane();

		move_drop_xbullets();
		collision_drop_xbullets();
*/
		show_player();

		show_bomb();
/*
		draw_toolbar();
*/
		flip(screen);
//		delay(1);
	}
	
	for(i = 0; i < NR_COUNT; i++)
	{
		free_bullet(&(handle[i]));
	}

	void resume_xbullets();
	resume_xbullets();
	void resume_score();
	resume_score();
	
	return;
}

void game_plot()
{
	//TODO:processing game by plot in plot.cpp
}

void game_start()
{
	resume_player();
	game_state = GAME_START;
	const uint32_t times = 3;

	int redraw = 1, sel = 0x0, count = 0;
	uint32_t color[] = {0x3f, 0x3f3f};
	uint32_t sel_color = 0x0;

	while(true)
	{
		if(redraw)
		{
			redraw = 0;
			update_screen();
			font_out((BK_W - 9 * times * 8) / 2, (BK_H - 2 * times * 8) / 2,
				0x3f, times, "plot mode");
			font_out((BK_W - 11 * times * 8) / 2, (BK_H + 2 * times * 8) / 2,
				0x3f, times, "random mode");

		}


		msgloop();
		switch(g_InputState.dir)
		{
			case kdirUp:
			case kdirLeft:
			case kdirDown:
			case kdirRight:
				sel ^= 0x1;
				if(sel & 0x1)
				{
					font_out((BK_W - 9 * times * 8) / 2, (BK_H - 2 * times * 8) / 2, color[0], times, "plot mode");
				}
				else
				{
					font_out((BK_W - 11 * times * 8) / 2, (BK_H + 2 * times * 8) / 2, color[0], times, "random mode");
				}
				break;
			default:break;
		}

		switch(sel & 0x1)
		{
			case 0:
				font_out((BK_W - 9 * times * 8) / 2, (BK_H - 2 * times * 8) / 2,
			color[sel_color], times, "plot mode");
				break;
			case 1:
				font_out((BK_W - 11 * times * 8) / 2, (BK_H + 2 * times * 8) / 2,
			color[sel_color], times, "random mode");
				break;
			default:break;
		}
		
		if(((count ++) & 0x3f) == 0)
			sel_color ^= 0x1;

		if(g_InputState.dwKey == KEY_RETURN)
		{
			if(sel & 0x1)
			{
				game_main();
				redraw = 1;
				game_state = GAME_START;
			}
			else
			{
				game_plot();
				redraw = 1;
				game_state = GAME_START;
			}
		}

		flip(screen);
		delay(1);
	}
	return;
}

void game_over()
{
	int jiffy = 0;
	game_state = GAME_END;
	while(jiffy ++ < 1500)
	{
		msgloop();
		update_screen();

		move_plane();
		show_plane();

		show_player();
		
		show_bomb();
		
		font_out((BK_W - 9 * 8 * 8) / 2, (BK_H - 8 * 8) / 2,
				0x3f, 8, "GAME OVER!");
		
		flip(screen);
		delay(1);
	}
}
