#include "common.h"
#include "bullet.h"
#include "item.h"

static float px = BK_W / 2;
static float py = BK_H - 40;

static int pw, ph, life;

/*玩家除飞机外，还应有子弹库，切换子弹功能
 */

#define NR_PLAYER_BULLETS 60

static HANDLE player_bullets[NR_PLAYER_BULLETS];
static POINT player_bullets_pos[NR_PLAYER_BULLETS];

/* stack pointer player bullets */
static uint32_t s_pb = 0;

/* define some kinds of bullets */
static IBS ibs_player[] = {
	{ID_ROUNDBULLET_0, ID_PLAYER, IDBF_SPEEDLINE, 3 * PI / 2, 3 * PI / 2 + 0.1, 1},
	{ID_ROUNDBULLET_0, ID_PLAYER, IDBF_SQUARE, PI + 0.2, 2 * PI, 1},
	{ID_ROUNDBULLET_0, ID_PLAYER, IDBF_OUTRIGHTSQUARE, PI, 3 * PI / 2, 1},
	{ID_ROUNDBULLET_0, ID_PLAYER, IDBF_OUTLEFTSQUARE, PI / 2, PI, 1},
	{ID_ROUNDBULLET_0, ID_PLAYER, IDBF_SPEEDLINE, 7 * PI / 6 + 0.25, 11 * PI / 6, 1},
	{ID_UPMISSILE, ID_PLAYER, IDBF_SPEEDLINE, 3 * PI / 2, 3 * PI / 2 + 0.1, 1},
	{ID_UPMISSILE, ID_PLAYER, IDBF_SQUARE, PI + 0.3, 2 * PI, 1},
};

#define NR_IBS_PLAYER (sizeof(ibs_player)/sizeof(ibs_player[0]))

/* function definition here */
static uint32_t query_empty_handle()
{
	uint32_t old_pb = s_pb;

	while(player_bullets[s_pb] != INVALID_HANDLE_VALUE)
	{
		s_pb = (s_pb + 1) % NR_PLAYER_BULLETS;
		if(old_pb == s_pb)
		{
			int i;
			for(i = 0; i < NR_PLAYER_BULLETS; i ++)
				printf("0x%x\t", player_bullets[i]);
			panic("player bullets aren't enough!");
		}
	}

	return s_pb;
}

void init_player()
{
	/* init player bullets */
	int i;

	px = BK_W / 2;
	py = BK_H - 40;

	POINT wh = query_itemwh(ID_PLAYER);

	pw = wh.x;
	ph = wh.y;

	life = query_life_atk(ID_PLAYER);

	for(i = 0; i < NR_PLAYER_BULLETS; i ++)
	{
		player_bullets[i] = INVALID_HANDLE_VALUE;
	}
}

void shot_player()
{
	if(life <= 0)
		return;
	static int times = 0;

	times ++;
	if(times == 15)
	{
		times = 0;
	}
	else
		return;

	uint32_t taridr = -1;
	int query_xbullets(char);
	void sub_xbullets(char, uint32_t);
	switch(g_InputState.dwKey)
	{
		case KEY_z:
			if(query_xbullets('z') > 0)
			{
				sub_xbullets('z', 1);
				taridr = query_empty_handle();
				player_bullets[taridr] = alloc_bullet(1);
				player_bullets_pos[taridr] = make_point(px, py);
				set_bullet(player_bullets[taridr], &ibs_player[0]);
			}
			break;
		case KEY_x:
			if(query_xbullets('x') > 0)
			{
				sub_xbullets('x', 1);
				taridr = query_empty_handle();
				player_bullets[taridr] = alloc_bullet(16);
				player_bullets_pos[taridr] = make_point(px, py);
				set_bullet(player_bullets[taridr], &ibs_player[1]);
			}
			break;
		case KEY_r:
			if(query_xbullets('r') > 0)
			{
				sub_xbullets('r', 1);
				taridr = query_empty_handle();
				player_bullets[taridr] = alloc_bullet(32);
				player_bullets_pos[taridr] = make_point(px, py);
				set_bullet(player_bullets[taridr], &ibs_player[2]);
			}
			break;
		case KEY_l:
			if(query_xbullets('l') > 0)
			{
				sub_xbullets('l', 1);
				taridr = query_empty_handle();
				player_bullets[taridr] = alloc_bullet(32);
				player_bullets_pos[taridr] = make_point(px, py);
				set_bullet(player_bullets[taridr], &ibs_player[3]);
			}
			break;
		case KEY_s:
			if(query_xbullets('s') > 0)
			{
				//sub_xbullets('s', 1);
				taridr = query_empty_handle();
				player_bullets[taridr] = alloc_bullet(6);
				player_bullets_pos[taridr] = make_point(px, py);
				set_bullet(player_bullets[taridr], &ibs_player[4]);
			}
			break;
		case KEY_a:
			//if(query_xbullets('s') > 0)
			//{
				//sub_xbullets('s', 1);
				taridr = query_empty_handle();
				player_bullets[taridr] = alloc_bullet(1);
				player_bullets_pos[taridr] = make_point(px, py);
				set_bullet(player_bullets[taridr], &ibs_player[5]);
			//}
			break;
		case KEY_w:
			//if(query_xbullets('s') > 0)
			//{
				//sub_xbullets('s', 1);
				taridr = query_empty_handle();
				player_bullets[taridr] = alloc_bullet(8);
				player_bullets_pos[taridr] = make_point(px, py);
				set_bullet(player_bullets[taridr], &ibs_player[6]);
			//}
			break;
		default:break;
	}
}

void move_player()
{
	int i;
	shot_player();
	//move player bullets
	for(i = 0; i < NR_PLAYER_BULLETS; i ++)
	{
		move_bullet(&(player_bullets[i]), player_bullets_pos[i]);
	}
	
	if(life <= 0)
		return;

	switch(g_InputState.dir)
	{
		case kdirUp:
			if(py - 2 - ph / 2 > 0)
				py -= 2;
			break;
		case kdirDown:
			if(py + 2 + ph / 2 < BK_H)
				py += 2;
			break;
		case kdirLeft:
			if(px - 2 - pw / 2 > 0)
				px -= 2;
			break;
		case kdirRight:
			if(px + 2 + pw / 2 < BK_W)
				px += 2;
			break;
		default:break;
	}
}

void show_player_bullets()
{
	/* move bullets firstly
	 */
	int i;
	for(i = 0; i < NR_PLAYER_BULLETS; i ++)
	{
		show_bullet(player_bullets[i]);
	}
}

void resume_player()
{
	int i;

	px = BK_W / 2;
	py = BK_H - 40;

	POINT wh = query_itemwh(ID_PLAYER);

	pw = wh.x;
	ph = wh.y;

	life = query_life_atk(ID_PLAYER);

	for(i = 0; i < NR_PLAYER_BULLETS; i ++)
	{
		free_bullet(&(player_bullets[i]));
	}
}

void kill_player()
{
	life = 0;
}

void show_player()
{
	show_player_bullets();
	if(life <= 0)
		return;
	show_item(ID_PLAYER, px, py);
}

POINT query_player_pos()
{
	return make_point(px, py);
}

int query_player_life()
{
	return life;
}

bool collision_player(uint32_t bulletID, POINT tarpos)
{
	if(tarpos.x > px - pw / 2
	&& tarpos.x < px + pw / 2
	&& tarpos.y > py - ph / 2
	&& tarpos.y < py + ph / 2)
	{
		if(query_item_collision(ID_PLAYER, tarpos.x - px + pw / 2, tarpos.y - py + ph / 2))
		{
			life -= query_life_atk(bulletID);
			if(life > 0)
			{
				add_bomb(tarpos, IDB_SMALL);
			}
			else
			{
				add_bomb(tarpos, IDB_BIG);
				void game_over();
				game_over();
			}
			return true;
		}
	}

	return false;
}
