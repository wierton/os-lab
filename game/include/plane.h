#ifndef __PLANE_H__
#define __PLANE_H__

#define NR_PLANE_HANDLE 40

enum ID_PLANE_FUNC {IDPF_LINE};

typedef struct tagPLANE_HELPER{
    POINT pos;
    float step;
    float dx, dy;
} PLANE_HELPER, *LPPLANE_HELPER;

typedef POINT (* PLANE_TRAIL_FUNC)(PLANE_HELPER *plane_helper);

typedef struct tagPLANE {
    int life, planeID, count, ti, nr_bullets;
	HANDLE handle[NR_PLANE_HANDLE];
    PLANE_TRAIL_FUNC plane_trail;
    PLANE_HELPER plane_helper;
	IBS ibs;
    struct tagPLANE * prev, *next;
} PLANE, *LPPLANE;

void init_plane();
LPPLANE alloc_plane();
void free_plane(LPPLANE target);
void move_plane();
void show_plane();
void set_plane(LPPLANE, uint32_t, uint32_t, LPPLANE_HELPER, uint32_t, LPIBS);
POINT query_plane_pos(LPPLANE target);

#endif
