#include "common.h"
#include "device/video.h"
#include "basic-font.h"
#include "string.h"
#include "syscall.h"

extern uint32_t _binary_data_bk_dat_start;
extern uint32_t _binary_data_it_dat_start;

#define CHAR_W 8
#define CHAR_H 8
#define SCR_W 640
#define SCR_H 480
#define SCR_SIZE (SCR_W * SCR_H)
#define SCR_BPL ((3 * SCR_W) & ~3)
#define SCR_BITSIZE (SCR_BPL * SCR_H)

#define NR_SURFACE 50
#define POOL_SIZE (SCR_W * SCR_H * 3 * 5)

static int pe = 0, pl = 0;
static SURFACE surface[NR_SURFACE];
static uint8_t pool[POOL_SIZE];

static SURFACE * canvas;

void flip(SURFACE *src)
{
	syscall(0, src->pixels);
}

SURFACE * load_bmp(int ID)
{
	uint8_t *dst;
	if(ID == IDB_BK)
		dst = (void *)(&_binary_data_bk_dat_start);
	else
		dst = (void *)(&_binary_data_it_dat_start);

	PBITMAPINFOHEADER bi = (PBITMAPINFOHEADER)((void *)dst + 14);

	assert(pe < NR_SURFACE);
	surface[pe].w = bi->biWidth;
	surface[pe].h = bi->biHeight;
	surface[pe].bpl = (3 * bi->biWidth + 3) & ~3;
	surface[pe].pixels = &pool[pl];
	surface[pe].is_video = false;

	int i, j;
	uint8_t * srcpixels = ((void *)dst + 54);
	uint32_t bitsize = surface[pe].bpl * surface[pe].h;

	for(i = 0; i < bi->biWidth; i++)
	{
		for(j = 0; j < bi->biHeight; j++)
		{
			uint8_t *src = srcpixels + 3 * i + j * surface[pe].bpl;
			uint8_t *dst = surface[pe].pixels + 3 * i + (bi->biHeight -1 - j) * surface[pe].bpl;
			dst[0] = src[0];
			dst[1] = src[1];
			dst[2] = src[2];
		}
	}

	pl += bitsize;
	pe ++;
	
	return &surface[pe - 1];
}

void blit_surface(SURFACE *src, RECT *srcrect, SURFACE *dst, RECT *dstrect)
{
	/* clip the area firstly
	 * */
	assert(dst && src);
	int j, sx = 0, sy = 0, dx = 0, dy = 0, w = src->w, h = src->h;
	if(srcrect != NULL)
	{
		sx = srcrect->x;
		sy = srcrect->y;
		w = srcrect->w;
		h = srcrect->h;
	}
	if(dstrect != NULL)
	{
		dx = dstrect->x;
		dy = dstrect->y;
		if(dx > dst->w || dx + w < 0 || dy > dst->h || dy + h < 0)
			return;
	}

	int ddx = max(0, dx);
	int ddy = max(0, dy);
	w = min(dx + w, dst->w) - ddx;
	h = min(dy + h, dst->h) - ddy;
	sx += ddx - dx;
	dx = ddx;
	sy += ddy - dy;
	dy = ddy;

//	if(w < 0 || h < 0 || sx > src->w || sy > src->h) return;

	int dsx = max(0, sx);
	int dsy = max(0, sy);
	w = min(sx + w, src->w) - dsx;
	h = min(sy + h, src->h) - dsy;
	dx += dsx - sx;
	sx = dsx;
	dy += dsy - sy;
	sy = dsy;
	
//	if(w < 0 || h < 0 || dx > dst->w || dy > dst->h) return;
//	printf("[%d, %d, %d, %d, %d, %d] ", sx, sy, dx, dy, w, h);

	int nw = 3 * w;
	uint8_t *sp = src->pixels + 3 * sx + sy * src->bpl;
	uint8_t *dp = dst->pixels + 3 * dx + dy * dst->bpl;
	for(j = 0; j < h; j ++)
	{		
#ifdef GAME_ACC
		asm volatile("movl %0, %%ecx"::"m"(nw));
		asm volatile("movl %0, %%esi"::"m"(sp));
		asm volatile("movl %0, %%edi"::"m"(dp));
		asm volatile("rep movsb");
		sp += src->bpl;
		dp += dst->bpl;
#else
		int i;
		for(i = 0; i < w; i ++)
		{
			if((((uint32_t *)sp)[0] | 0xff000000) != src->keycolor)
			{
				dp[0] = sp[0];
				dp[1] = sp[1];
				dp[2] = sp[2];
			}
			assert((void*)dp < (dst->pixels + dst->bpl * dst->h));
/*			if((void*)dp < (dst->pixels))
			{
				printf("[%d, %d, %d, %d, %d, %d, %d, %d, %d, %d] ", sx, sy, dx, dy, w, h, ddx, ddy, dsx, dsy);
			}
*/			assert((void*)dp >= (dst->pixels));
			sp += 3;
			dp += 3;
		}
		sp = sp - nw + src->bpl;
		dp = dp - nw + dst->bpl;
#endif
	}
}

SURFACE *create_surface(uint32_t w, uint32_t h)
{
	uint32_t bytes = (3 * w + 3) & ~3;
	uint32_t size = bytes * h;

	assert(pe < NR_SURFACE);
	assert(pl + size < POOL_SIZE);
	surface[pe].w = w;
	surface[pe].h = h;
	surface[pe].bpl = (3 * w + 3) & ~3;
	surface[pe].is_video = false;
	surface[pe].pixels = &pool[pl];
	pe ++;
	pl += size;

	return &surface[pe - 1];
}

void set_colorkey(SURFACE *src, uint32_t colorkey)
{
	src->keycolor = colorkey | 0xff000000;
}

SURFACE * set_videomode()
{
	assert(pe < NR_SURFACE);
	assert(pl + SCR_BITSIZE < POOL_SIZE);
	surface[pe].w = SCR_W;
	surface[pe].h = SCR_H;
	surface[pe].bpl = (3 * SCR_W + 3) & ~3;
	surface[pe].is_video = true;
	surface[pe].pixels = &pool[pl];
	pe ++;
	pl += SCR_BITSIZE;

	canvas = &surface[pe - 1];
	return &surface[pe - 1];
}

void font_sin(char ch, int x, int y, uint32_t color, uint8_t times)
{
	uint32_t i, j;
	
	for(i = 0; i < CHAR_W * times; i++)
		for(j = 0; j < CHAR_H * times; j++)
		{
			uint32_t tar = 3 * (x + i) + (y + j) * (3 * BK_W);
			uint32_t *vmem = (uint32_t *)(canvas->pixels + tar);
			if((font8x8_basic[(uint8_t)ch][j / times] >> (i / times)) & 0x1)
			{
				vmem[0] = color;
			}
		}

}

void __attribute__((noinline)) font_out(int x, int y, uint32_t color, uint8_t times, const char *ctl, ...)
{
	char str[100];
	vsprintf(str, ctl, ((void **)&ctl) + 1);
	int i, len = strlen(str);
	for(i = 0; i < len; i++)
	{
		font_sin(str[i], x + i * times * CHAR_H, y, color, times);
	}
}
