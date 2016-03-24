#include "common.h"
#include "bullet.h"


/******************************************************************
 * this seems to be stupid, the main aim is to study the principle*
 * of dynamic memory alloc in real OS.                            *
 ******************************************************************/

/* the number of total bullets which can be alloc */
#define ADDR_WIDTH 15
#define OFF_WIDTH  5
#define DIR_WIDTH  (ADDR_WIDTH - OFF_WIDTH)

#define NR_BULLET (1 << ADDR_WIDTH)
#define BLOCK_SIZE (1 << OFF_WIDTH)
#define NR_BLOCK (NR_BULLET / BLOCK_SIZE)

#define OFF_MASK (BLOCK_SIZE - 1)

bool collision_plane(uint32_t, POINT pos);
bool collision_player(uint32_t, POINT tarpos);

/* the definition of bullet */

static POINT line(POINT plane_pos, BULLET_HELPER * bullet_helper)
{
    float d = bullet_helper->step;
    bullet_helper->step += 0.5;
    return make_point(plane_pos.x + d * bullet_helper->dx, plane_pos.y + d * bullet_helper->dy);
}

static POINT speedline(POINT plane_pos, BULLET_HELPER * bullet_helper)
{
    float d = bullet_helper->step;
    bullet_helper->step += 1.2;
    return make_point(plane_pos.x + d * bullet_helper->dx, plane_pos.y + d * bullet_helper->dy);
}

static POINT square(POINT plane_pos, BULLET_HELPER * bullet_helper)
{
	float d = bullet_helper->step;
	bullet_helper->step += 0.5;
	return make_point(plane_pos.x + d * bullet_helper->dx, plane_pos.y + d * d * bullet_helper->dy / 8);
}

static POINT outrightsquare(POINT plane_pos, BULLET_HELPER * bullet_helper)
{
	float d = bullet_helper->step;
	float d2 = d * d / 64;
	bullet_helper->step += 0.5;
	return make_point(plane_pos.x + d * bullet_helper->ctheta - d2 * bullet_helper->stheta,
			plane_pos.y + d * bullet_helper->stheta + d2 * bullet_helper->ctheta);
}

static POINT outleftsquare(POINT plane_pos, BULLET_HELPER * bullet_helper)
{
	float d = bullet_helper->step;
	float d2 = d * d / 64;
	bullet_helper->step += 0.5;
	return make_point(plane_pos.x - d * bullet_helper->ctheta - d2 * bullet_helper->stheta,
			plane_pos.y - d * bullet_helper->stheta + d2 * bullet_helper->ctheta);
}

static BULLET_TRAIL_FUNC bullet_trail_func[] = {line, speedline, square, outrightsquare, outleftsquare};

#define NR_BULLET_FUNC (sizeof(bullet_trail_func)/sizeof(bullet_trail_func[0]))

/* some definition of map between map and bullet */
typedef struct tagMAP {
	bool empty;
	int life;
	uint32_t len;
	uint32_t dir : DIR_WIDTH;
} MAP, *LPMAP;

static MAP map[NR_BLOCK];

static bool line_valid[NR_BLOCK];
static BULLET bullet[NR_BLOCK][BLOCK_SIZE];

static uint32_t _edata;

POINT judge_collision()
{
	uint32_t i, j;
	for(i = 0; i < _edata; i++)
	{
		for(j = 0; j < map[i].len; j++)
		{
			assert(i + j / BLOCK_SIZE < NR_BLOCK);
			uint32_t dir = map[i + j / BLOCK_SIZE].dir;
			uint32_t off = j & OFF_MASK;
			uint32_t bulletID = bullet[dir][off].bulletID;
			if(bullet[dir][off].life > 0)
			{
				if(bullet[dir][off].planeID == ID_PLAYER)
				{
					//query enemy
					if(collision_plane(bulletID, bullet[dir][off].pos))
						bullet[dir][off].life = 0;
				}
				else
				{
					//query player
					if(collision_player(bulletID, bullet[dir][off].pos))
					{
						bullet[dir][off].life = 0;
					}
				}
			}
		}
	}
	return make_point(-1, -1);
}

void init_bullet()
{
    int i, j;
	_edata = 0;    
    /* init the len */
    for(i = 0; i < NR_BLOCK; i++)
    {
		for(j = 0; j < BLOCK_SIZE; j++)
			bullet[i][j].bullet_trail = NULL;
		map[i].empty = true;
		map[i].len = 0;
		line_valid[i] = true;
    }
}

/* function for alloc and free bullet */

/* this function is not suggested to be used */
LPBULLET get_bullet(HANDLE handle)
{
	if(handle == INVALID_HANDLE_VALUE)
		return NULL;
	uint32_t off = handle & OFF_MASK;
	uint32_t dir = handle >> OFF_WIDTH;
	assert(dir < NR_BLOCK);
	assert(!(map[dir].empty));

	assert(!(line_valid[map[dir].dir]));
	
	return &bullet[map[dir].dir][off];
}

HANDLE create_map(uint32_t size)
{
	uint32_t i, count = 0;
	int mappos = -1;
	uint32_t nBlocks = ((size + OFF_MASK) >> OFF_WIDTH);
	for(i = 0; i < _edata; i++)
	{
		if(map[i].empty)
		{
			count ++;
			if(count >= nBlocks)
			{
				mappos = i - count + 1;
				break;
			}
		}
		else
		{
			count = 0;
		}
	}
	if(mappos == -1)
	{
		mappos = _edata;
		_edata += nBlocks;
	}

	count = 0;
	for(i = 0; i < NR_BLOCK; i++)
	{
		if(line_valid[i])
		{
			line_valid[i] = false;
			map[mappos + count].empty = false;
			map[mappos + count].dir = i;
			count ++;
			if(count >= nBlocks)
			{
				map[mappos].life = nBlocks;
				map[mappos].len = size;
				return (mappos << OFF_WIDTH);
			}
		}
	}
	panic("no enough bullets for alloc!\n");
	return INVALID_HANDLE_VALUE;
}

HANDLE alloc_bullet(size_t size)
{
	if(size == 0)
		return INVALID_HANDLE_VALUE;
	return create_map(size);
}

/* should be free naturally but not manually */
void free_bullet(HANDLE * handle)
{
	if(*handle == INVALID_HANDLE_VALUE)
		return;
	uint32_t dir = (*handle >> OFF_WIDTH);
	assert(dir < NR_BLOCK);
	assert(!(map[dir].empty));

	uint32_t i;
	uint32_t nBlocks = ((map[dir].len + OFF_MASK) >> OFF_WIDTH);
	map[dir].len = 0;
	for(i = 0; i < nBlocks; i++)
	{
		if(!(map[i + dir].empty))
		{
			uint32_t tardir = map[i + dir].dir;
			line_valid[tardir] = true;
			map[i + dir].empty = true;
		}
	}
	*handle = INVALID_HANDLE_VALUE;

	for(;map[_edata].empty; _edata--);
	_edata ++;
}

/* a function for testing alloc and free */
void test_bullet()
{
	int i;
	init_bullet();
	uint32_t tmp1 = alloc_bullet(60);
	for(i = 0; i < 60; i++)
		get_bullet(tmp1 + i);
	free_bullet(&(tmp1));
	//for(i = 0; i < 60; i++)
	//	get_bullet(tmp1 + i);

	uint32_t tmp2 = alloc_bullet(128);
	for(i = 0; i < 128; i++)
		get_bullet(tmp2 + i);

}

void set_bullet(HANDLE handle, LPIBS lpibs)
{
	if(handle == INVALID_HANDLE_VALUE)
		return;
	uint32_t dir = (handle >> OFF_WIDTH);
	Assert(dir < NR_BLOCK, "Invalid handle 0x%x\n", handle);
	assert(!(map[dir].empty));

	uint32_t i, movlen = map[dir].len;
	float init_step = lpibs->init_step;
	float op_theta = lpibs->op_theta;
	float ed_theta = lpibs->ed_theta;
	assert(ed_theta >= op_theta);
	assert(movlen != 0);
	float d_theta = ((ed_theta - op_theta) / movlen);
	for(i = 0; i < movlen; i++)
	{
		LPBULLET tarbullet = get_bullet(handle + i);
		
		tarbullet->life = 1;
		tarbullet->planeID = lpibs->planeID;

		tarbullet->bullet_helper.step = init_step;
		float ctheta = cos(op_theta + i * d_theta);
		float stheta = sin(op_theta + i * d_theta);

		tarbullet->bullet_helper.dx = init_step * ctheta;
		tarbullet->bullet_helper.dy = init_step * stheta;


		tarbullet->bullet_helper.ctheta = ctheta;
		tarbullet->bullet_helper.stheta = stheta;

		tarbullet->bulletID = lpibs->bulletID;
		assert(lpibs->funcID < NR_BULLET_FUNC);
		tarbullet->bullet_trail = bullet_trail_func[lpibs->funcID];
	}
}

/* function for moving bullet */

void move_bullet(HANDLE * handle, POINT plane_pos)
{
	if(*handle == INVALID_HANDLE_VALUE)
		return;
	uint32_t dir = (*handle >> OFF_WIDTH);
	assert(dir < NR_BLOCK);
	assert(!(map[dir].empty));

	uint32_t live = 0;
	uint32_t i, movlen = map[dir].len;
	for(i = 0; i < movlen; i++)
	{
		LPBULLET tarbullet = get_bullet(*handle + i);
		assert(tarbullet->bullet_trail != NULL);
		POINT pos = tarbullet->bullet_trail(plane_pos, &(tarbullet->bullet_helper));
		tarbullet->pos = pos;
		if(tarbullet->life > 0)
			live += !is_offscreen(tarbullet->bulletID, pos);
	}
	if(live == 0)
	{
		/* 选择一：死亡
		 * 选择二：重生
		 */
		//set_bullet();
		free_bullet(handle);
	}
}


void show_bullet(HANDLE handle)
{
	if(handle == INVALID_HANDLE_VALUE)
		return;
	uint32_t dir = (handle >> OFF_WIDTH);
	assert(dir < NR_BLOCK);
	assert(!(map[dir].empty));

	uint32_t i, showlen = map[dir].len;
	for(i = 0; i < showlen; i++)
	{
		LPBULLET tarbullet = get_bullet(handle + i);
		if(tarbullet->life > 0)
		{
			POINT pos = tarbullet->pos;
			show_item(tarbullet->bulletID, pos.x, pos.y);
		}
	}
}
