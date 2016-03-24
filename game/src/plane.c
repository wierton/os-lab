#include "common.h"
#include "bullet.h"
#include "plane.h"
#include "item.h"

/*飞机（敌机）的组分：机身、子弹
 */

/* the number of total NR_PLANE which can be alloc */
/*同屏敌机数量上限:20
 */
#define NR_PLANE 20

/* the definition of plane */

POINT line(PLANE_HELPER *plane_helper)
{
    plane_helper->step += 0.5;
    plane_helper->pos.y += 0.5;
    return make_point(plane_helper->pos.x, plane_helper->pos.y);
}

PLANE_TRAIL_FUNC plane_trail_func[] = {line};

#define NR_PLANE_FUNC (sizeof(plane_trail_func)/sizeof(plane_trail_func[0]))

static PLANE plane[NR_PLANE], *empty, *full;
static int tti = 0;

void init_plane()
{
    int i, j;
	tti = 0;
    
    /* init member varible */
    for(i = 0; i < NR_PLANE; i++)
    {
        plane[i].plane_helper.dx = 0;
        plane[i].plane_trail = NULL;
		plane[i].life = 0;
		plane[i].ti = 0;
		plane[i].nr_bullets = 0;
		for(j = 0; j < NR_PLANE_HANDLE; j++)
		{
			plane[i].handle[j] = INVALID_HANDLE_VALUE;
		}
    }
    
    /* construct a list for plane */
    plane[0].prev = NULL;
    for(i = 0; i < NR_PLANE - 1; i++)
    {
        plane[i].next = &plane[i + 1];
        plane[i + 1].prev = &plane[i];
    }
    plane[i].next = NULL;
    
    /**/
    empty = &plane[0];
    full = NULL;
}

/* function for alloc and free plane */

LPPLANE alloc_plane()
{
    if(empty != NULL)
    {
        LPPLANE tmp = empty;
		Assert(tmp != NULL, "no enough plane!\n");
        empty = empty->next;
		if(empty != NULL)
			empty->prev = NULL;
        tmp->next = full;
        if(full != NULL)
			full->prev = tmp;
        full = tmp;
        tmp->prev = NULL;
        return tmp;
    }
	panic("no enough plane");
    return NULL;
}

void free_plane(LPPLANE target)
{
    Assert((target < plane + NR_PLANE) && (target >= plane), "free plane not valid!\n");
	LPPLANE tmp = target;
	LPPLANE target_next = target->next;
    LPPLANE prev = target->prev;
		
	target->life = 0;
	target->ti = 0;
	target->nr_bullets = 0;

	target->life = 0;

	if(prev != NULL)
		target->prev->next = target->next;
	if(target->next != NULL)
		target->next->prev = prev;
//这里需要检查    
    target->prev = NULL;
    target->next = empty;
	if(empty != NULL)
		empty->prev = target;
    empty = target;
	if(tmp == full)
		full = target_next;
}

void move_plane()
{
	int i, j, tj;
	tti ++;
	for(i = 0; i < NR_PLANE; i++)
	{
		tj = -1;
		for(j = 0; j < plane[i].nr_bullets; j++)
		{
			move_bullet(&(plane[i].handle[j]), plane[i].plane_helper.pos);
			if(plane[i].life > 0 && plane[i].handle[j] == INVALID_HANDLE_VALUE)
			{
				tj = j;
			}
		}
		if((tti & 0x1f) == 0 && tj != -1)
		{
			plane[i].handle[tj] = alloc_bullet(plane[i].nr_bullets);
			set_bullet(plane[i].handle[tj], &(plane[i].ibs));
		}
	}

	LPPLANE tmp = full;
	while(tmp != NULL)
	{
		assert(tmp->plane_trail != NULL);
		POINT newpos = tmp->plane_trail(&(tmp->plane_helper));
		tmp->plane_helper.pos = newpos;

		LPPLANE free_tmp = tmp;
		tmp = tmp->next;
		//free it if off screen
		if(is_offscreen(free_tmp->planeID, free_tmp->plane_helper.pos))
			free_plane(free_tmp);
	}
}

void set_plane(LPPLANE target, uint32_t planeID, uint32_t funcID, LPPLANE_HELPER lpplane_helper, uint32_t nr_bullets, LPIBS lpibs)
{
	target->plane_helper = *lpplane_helper;	
	set_bullet(target->handle[0], lpibs);
	target->plane_trail = plane_trail_func[funcID];
	target->life = query_life_atk(planeID);
	target->planeID = planeID;
	target->nr_bullets = nr_bullets;
	target->ibs = *lpibs;
}

void show_plane()
{
	int i, j;
	LPPLANE tmp = full;

	for(i = 0; i < NR_PLANE; i++)
	{
		for(j = 0; j < plane[i].nr_bullets; j++)
		{
			show_bullet(plane[i].handle[j]);
		}
	}

	while(tmp != NULL)
	{
		if(tmp->life > 0)
			show_item(tmp->planeID, tmp->plane_helper.pos.x, tmp->plane_helper.pos.y);
		tmp = tmp->next;
	}
}

bool collision_plane(uint32_t bulletID, POINT bullet_pos)
{
	LPPLANE tmp = full;
	while(tmp != NULL)
	{
		POINT wh = query_itemwh(tmp->planeID);
		POINT pos = tmp->plane_helper.pos;
		int w = wh.x, h = wh.y;
		if(bullet_pos.x > pos.x - w / 2
		&& bullet_pos.x < pos.x + w / 2
		&& bullet_pos.y > pos.y - h / 2
		&& bullet_pos.y < pos.y + h / 2)
		{
			if(tmp->life > 0 && query_item_collision(tmp->planeID, bullet_pos.x - pos.x + w / 2, bullet_pos.y - pos.y + h / 2))
			{
				uint32_t atk = query_life_atk(bulletID);
				void add_score(int);
				add_score(query_life_atk(tmp->planeID));
				tmp->life -= atk;
				if(tmp->life > 0)
					add_bomb(bullet_pos, IDB_SMALL);
				else
					add_bomb(bullet_pos, IDB_BIG);
				return true;
			}
		}
		tmp = tmp->next;
	}
	return false;
}

POINT query_plane_pos(LPPLANE target)
{
	return target->plane_helper.pos;
}
