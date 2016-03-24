#include "common.h"
#include "bullet.h"
#include "plane.h"
#include "bomb.h"

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

void init_player();
void init_plane();
void init_bullet();
void init_bomb();
void init_xbullets();
void add_drop_xbullets(char ch);
void rand_drop_xbullets();
void move_drop_xbullets();
void collision_drop_xbullets();
void show_player();
void draw_toolbar();
void delay(uint32_t ms);
int read_menu();

extern SURFACE *screen;

int count = 0;

void game_main()
{
	int i;
	const int NR_COUNT = 40;
	HANDLE handle[NR_COUNT];
	
	IBS ibs_small = {ID_ROUNDBULLET_0, ID_ENEMY_SMALL, IDBF_SPEEDLINE, PI / 3 + 0.25, 2 * PI / 3, 1};
	IBS ibs_medium = {ID_DOWNMISSILE, ID_ENEMY_MEDIUM, IDBF_SPEEDLINE, 3 * PI / 8 + 0.25, 5 * PI / 8, 1};
	
	for(i = 0; i < NR_COUNT; i++)
	{
		handle[i] = alloc_bullet(13);
		set_bullet(handle[i], &ibs_small);
	}

	PLANE_HELPER plane_helper = {make_point(100, 80), 1, 0, 0};

	i = 0x3e;
	while(game_state != GAME_END)
	{
		i++;
		judge_collision();
		if((i & 0x3f) == 0)
		{
			LPPLANE newplane = alloc_plane();
			plane_helper.pos.x = rand() % (BK_W - 100) + 50;
			plane_helper.pos.y = 1;
			set_plane(newplane, ID_ENEMY_SMALL, IDPF_LINE, &plane_helper, 3, &ibs_small);
		}
		if((i & 0xff) == 0)
		{
			LPPLANE newplane = alloc_plane();
			plane_helper.pos.x = rand() % (BK_W - 100) + 50;
			set_plane(newplane, ID_ENEMY_MEDIUM, IDPF_LINE, &plane_helper, 3, &ibs_medium);
		}
		if((i & 0x3ff) == 0)
		{
			rand_drop_xbullets();
		}

		msgloop();
		update_screen();

		move_plane();
		show_plane();

		move_drop_xbullets();
		collision_drop_xbullets();

		show_player();

		show_bomb();

		draw_toolbar();

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
	//TODO:processing game by plot in plot.c
}

void game_start()
{
	while(true)
	{
		init_player();
		init_plane();
		init_bullet();
		init_bomb();
		init_xbullets();
		game_state = GAME_START;

		int sel = read_menu();

		if(sel & 0x1)
		{
			game_plot();
		}
		else
		{
			game_main();
		}

		delay(1);
	}
	return;
}

void game_over()
{
	int jiffy = 0;
	game_state = GAME_END;
	while(jiffy ++ < 400)
	{
		msgloop();
		update_screen();

		move_plane();
		show_plane();

		show_player();
		
		show_bomb();
		
		font_out((BK_W - 9 * 8 * 8) / 2, (BK_H - 8 * 8) / 2,
				0x3f, 7, "GAME OVER!");
		
		flip(screen);
		delay(1);
	}
}
