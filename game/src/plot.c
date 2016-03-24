#include "common.h"
#include "bullet.h"
#include "plane.h"
#include "plot.h"

PLOT plot [] = {
	{{ID_ROUNDBULLET_0, ID_ENEMY_SMALL, IDBF_SPEEDLINE, PI / 3 + 0.25, 2 *PI / 3, 1}, ID_ENEMY_SMALL, 3, IDPF_LINE, {{100, 1}, 1, 0, 0}, 0}
};

#define NR_PLOT (sizeof(plot) / sizeof(plot[0]))
