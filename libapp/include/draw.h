#ifndef __DRAW_H__
#define __DRAW_H__

#define SCR_W 640
#define SCR_H 480
#define SCR_SIZE (SCR_W * SCR_H)
#define SCR_BPL ((3 * SCR_W) & ~3)
#define SCR_BITSIZE (SCR_BPL * SCR_H)

void invalidate();
int draw_rect(uint32_t color, int x, int y, uint32_t w, uint32_t h);

int __attribute__((noinline)) draw_str(int x, int y, uint32_t color, uint8_t times, const char *ctl, ...);

#endif
