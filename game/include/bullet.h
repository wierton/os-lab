#ifndef __BULLET_H__
#define __BULLET_H__

enum ID_BULLETFUNC {IDBF_LINE, IDBF_SPEEDLINE, IDBF_SQUARE, IDBF_OUTRIGHTSQUARE, IDBF_OUTLEFTSQUARE};

typedef struct{
    float step;
    union {
        struct{float dx, dy;};
        struct{float ctheta, stheta;};
    };
} BULLET_HELPER;

typedef POINT (* BULLET_TRAIL_FUNC)(POINT, BULLET_HELPER *);

typedef struct tagBULLET {
    int life;
	uint32_t bulletID, planeID;
	POINT pos;
    BULLET_TRAIL_FUNC bullet_trail;
    BULLET_HELPER bullet_helper;
} BULLET, *LPBULLET;

/* initial bullet state */
typedef struct tagIBS {
	uint32_t bulletID, planeID, funcID;
	float op_theta, ed_theta;
	float init_step;
} IBS, *LPIBS;

void init_bullet();
HANDLE alloc_bullet(size_t);
void free_bullet(HANDLE *);
void set_bullet(HANDLE, LPIBS);
void move_bullet(HANDLE *, POINT);
void show_bullet(HANDLE);
POINT judge_collision();

#endif
