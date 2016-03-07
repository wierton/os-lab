#include "body/game-common.h"
#include "body/bomb.h"

/* 爆炸的特性，先炸的先结束，后炸的后结束
 * 适用队列来描述
 */

static BOMBINFO bombinfo[100];

static int op, ed;

void init_bomb()
{
	int i;
	op = ed = 0;
	for(i = 0; i < NR_BOMB; i++)
	{
		bombinfo[i].p = -1;
	}
}

void add_bomb(POINT pos, BOMB_TYPE bomb_type)
{
	bombinfo[ed].pos = pos;
	bombinfo[ed].p = 0;
	bombinfo[ed].bomb_type = bomb_type;
	ed = (ed + 1) % NR_BOMB;
}

void show_bomb()
{
	int tmp = op;
	while(tmp != ed)
	{
		if(bombinfo[tmp].p >= 8 * NR_DELAYPERCALL)
		{
			op = (op + 1) % NR_BOMB;
			bombinfo[tmp].p = -1;
		}
		else
		{
			switch(bombinfo[tmp].bomb_type)
			{
				case IDB_SMALL:
					show_item(ID_SMALLBOMB_0 + bombinfo[tmp].p / NR_DELAYPERCALL, bombinfo[tmp].pos.x, bombinfo[tmp].pos.y);
					break;
				case IDB_BIG:
					show_item(ID_BIGBOMB_0 + bombinfo[tmp].p / NR_DELAYPERCALL, bombinfo[tmp].pos.x, bombinfo[tmp].pos.y);
					break;
				default:
					panic("no such bomb!\n");
					break;
			}

			bombinfo[tmp].p ++;
		}
		tmp = (tmp + 1) % NR_BOMB;
	}
}

