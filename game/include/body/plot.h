#ifndef __PLOT_H__
#define __PLOT_H__

typedef struct tagPLOT {
	IBS ibs;
	uint32_t planeID;
	uint32_t nr_bullets;
	uint32_t funcID;
	PLANE_HELPER plane_helper;
	uint32_t next_milisec;
} PLOT, LPPLOT;


#endif
