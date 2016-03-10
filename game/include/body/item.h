#ifndef __ITEM_H__
#define __ITEM_H__

#define BLOOD_W 50
#define BLOOD_H 16

enum ID_ITEM {
	ID_PLAYER,
	ID_ENEMY_SMALL,
	ID_ENEMY_MEDIUM,
	ID_ENEMY_BIG,
	ID_ROUNDBULLET_0,
	ID_ROUNDBULLET_1,
	ID_BLOOD,
	ID_DOWNMISSILE,
	ID_UPMISSILE,
	ID_SMALLBOMB_0,
	ID_SMALLBOMB_1,
	ID_SMALLBOMB_2,
	ID_SMALLBOMB_3,
	ID_SMALLBOMB_4,
	ID_SMALLBOMB_5,
	ID_SMALLBOMB_6,
	ID_SMALLBOMB_7,
	ID_BIGBOMB_0,
	ID_BIGBOMB_1,
	ID_BIGBOMB_2,
	ID_BIGBOMB_3,
	ID_BIGBOMB_4,
	ID_BIGBOMB_5,
	ID_BIGBOMB_6,
	ID_BIGBOMB_7,
	ID_CHAR
};

typedef struct {
    float x, y;
} POINT;

inline POINT make_point(float a, float b);

typedef POINT (*CALC_TRAIL_FUNC)(POINT, POINT, POINT);

bool is_offscreen(uint32_t itemID, POINT pos);
void show_item(uint32_t, float, float);
POINT query_itemwh(uint32_t);
bool query_item_collision(uint32_t, int, int);
uint32_t query_life_atk(uint32_t);
void draw_strip(int, uint32_t, int, int);

#endif
