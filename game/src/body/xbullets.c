#include "body/game-common.h"

static int score = 0;

static int xbullets[128] = {0};

static uint32_t xbullets_bound[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,//U + 0x00
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,//U + 0x10
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,//U + 0x20
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,//U + 0x30
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,//U + 0x40
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,//U + 0x50
//char start
//  a	b	c	d	e	f	g	h
	0,	0,	0,	0,	0,	0,	0,	0,
//	i	j	k	l	m	n	o	p
	0,	0,	0,	60,	0,	0,	0,	0,//U + 0x60
//	q	r	s	t	u	v	w	x
	0,	60,	INF,0,	0,	0,	0,	1000,
//	y	z
	0,	INF,0,	0,	0,	0,	0,	0//U + 0x70
};

static char xbullets_left [] = {'x', 'r', 'l'};

#define NR_XBULLETS_LEFT (sizeof(xbullets_left)) / sizeof(xbullets_left[0])

int query_player_life();

void init_xbullets()
{
	xbullets_bound[(uint8_t)'x'] = 200;
	xbullets_bound[(uint8_t)'r'] = 60;
	xbullets_bound[(uint8_t)'l'] = 60;
	xbullets_bound[(uint8_t)'z'] = INF;
	xbullets_bound[(uint8_t)'s'] = INF;
	xbullets[(uint8_t)'x'] = 200;
	xbullets[(uint8_t)'r'] = 0;
	xbullets[(uint8_t)'l'] = 0;
	xbullets[(uint8_t)'z'] = INF;
	xbullets[(uint8_t)'s'] = INF;
}

void resume_xbullets()
{
	init_xbullets();
}

int query_xbullets(char ch)
{
	uint8_t dir = (uint8_t)ch;
	return xbullets[dir];
}

void add_xbullets(char ch)
{
	uint8_t dir = (uint8_t)ch;
	xbullets[dir] = xbullets_bound[dir];
}

void sub_xbullets(char ch, uint32_t sub)
{
	uint8_t dir = (uint8_t)ch;
	if(xbullets_bound[dir] == INF)
		return;
	if(xbullets[dir] - sub <= 0)
		xbullets[dir] = 0;
	else
		xbullets[dir] -= sub;
}

void set_xbullets_bound(char ch, int set_val)
{
	uint8_t dir = (uint8_t)ch;
	xbullets_bound[dir] = set_val;
}


#define NR_DROP_XBULLETS 100

typedef struct tagDROP_XBULLETS {
	POINT pos;
	bool dead;
	char ch;
} DROP_XBULLETS, *LPDROP_XBULLETS;

static DROP_XBULLETS drop_xbullets[NR_DROP_XBULLETS];

static int op_drop_xbullets = 0, ed_drop_xbullets = 0;

void add_drop_xbullets(char ch)
{
	uint32_t i;
	for(i = 0; i < NR_XBULLETS_LEFT; i++)
	{
		if(ch == xbullets_left[i])
		{
			drop_xbullets[ed_drop_xbullets].pos = make_point(rand() % BK_W, 1);
			drop_xbullets[ed_drop_xbullets].ch = ch;
			drop_xbullets[ed_drop_xbullets].dead = false;
			ed_drop_xbullets = (ed_drop_xbullets + 1) % NR_DROP_XBULLETS;
		}
	}
}

void rand_drop_xbullets()
{
	char ch = xbullets_left[rand() % NR_XBULLETS_LEFT];

	drop_xbullets[ed_drop_xbullets].pos = make_point(rand() % BK_W, 1);
	drop_xbullets[ed_drop_xbullets].ch = ch;
	drop_xbullets[ed_drop_xbullets].dead = false;
	ed_drop_xbullets = (ed_drop_xbullets + 1) % NR_DROP_XBULLETS;
}

void move_drop_xbullets()
{
	int i = op_drop_xbullets;
	while(i != ed_drop_xbullets)
	{
		drop_xbullets[i].pos.y += 0.5;
		if(!(drop_xbullets[i].dead))
		{
			if(drop_xbullets[i].pos.y < BK_H)
				font_out(drop_xbullets[i].pos.x, drop_xbullets[i].pos.y, 0x3fa03f, 2, "%c", drop_xbullets[i].ch);
			else
			{
				op_drop_xbullets ++;
				drop_xbullets[i].dead = true;
			}
		}
		i = (i + 1) % NR_DROP_XBULLETS;
	}
}

void collision_drop_xbullets()
{
	int i = op_drop_xbullets;
	POINT query_player_pos();
	POINT query_itemwh(uint32_t);
	POINT pos = query_player_pos();
	POINT wh = query_itemwh(ID_PLAYER);
	while(i != ed_drop_xbullets)
	{
		POINT tarpos = drop_xbullets[i].pos;
		if(!(drop_xbullets[i].dead)
		&& tarpos.x > pos.x - wh.x / 2
		&& tarpos.x < pos.x + wh.x / 2
		&& tarpos.y > pos.y - wh.y / 2
		&& tarpos.y < pos.y + wh.y / 2
		)
		{
	//		if(query_item_collision(ID_PLAYER, tarpos.x - pos.x + wh.x / 2, tarpos.y - pos.y + wh.y / 2))
	//		{
				drop_xbullets[i].dead = true;
				add_xbullets(drop_xbullets[i].ch);
	//		}
		}
		i = (i + 1) % NR_DROP_XBULLETS;
	}
}

void add_score(int plus)
{
	score += plus;
}

void resume_score()
{
	score = 0;
}

void draw_toolbar()
{
	uint32_t i, j = 4;
	const int dist = 25;
	font_out(BK_W - 200, dist * 1, 0x3f, 2, "score:%d", score);
	font_out(BK_W - 200, dist * 2, 0x3f, 2, "life:%d/%d", query_player_life(), query_life_atk(ID_PLAYER));
	font_out(BK_W - 200, dist * 3, 0x3f, 2, "w a s z x c");
	
	for(i = 0; i < NR_XBULLETS_LEFT; i++)
	{
		uint8_t dir = (uint8_t)xbullets_left[i];
		if(xbullets[dir] > 0)
		{
			font_out(BK_W - 200, dist * (j ++), 0x3f, 2, "%c:%d", xbullets_left[i], xbullets[dir]);
		}
	}
}
