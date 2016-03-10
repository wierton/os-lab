#ifndef __BOMB_H__
#define __BOMB_H__

#define NR_BOMB 100

#define NR_DELAYPERCALL 3

typedef enum {IDB_SMALL, IDB_BIG} BOMB_TYPE;

typedef struct tagBOMBINFO {
	POINT pos;
	int p;
	BOMB_TYPE bomb_type;
} BOMBINFO, *LPBOMBINFO;

void add_bomb(POINT pos, BOMB_TYPE bomb_type);
void show_bomb();

#endif
