#ifndef __GAME_COMMON_H__
#define __GAME_COMMON_H__

#include "type.h"

#include "math.h"

#include "device/video.h"
#include "font/font.h"
#include "surface.h"
#include "debug.h"
#include "item.h"
#include "msg.h"
#include "bomb.h"

#define BK_W 640
#define BK_H 480

#define INVALID_SCANCODE 0xffffffff

#define is_inbounder(x, y) ((x < BK_W) && (x >= 0) && (y < BK_H) && (y >= 0))

typedef uint32_t HANDLE;

static inline float dist(POINT a, POINT b)
{
    float dx = fabs(a.x-b.x);
    float dy = fabs(a.y-b.y);
    return sqrt(dx*dx+dy*dy);
}

#define INF 0x7fffffff

#define INVALID_HANDLE_VALUE 0xffffffff

#define PI 3.1415926

#define E 2.71828

void update_screen();

#endif
