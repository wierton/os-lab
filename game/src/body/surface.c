#include "body/game-common.h"
#include "device/video.h"

extern uint32_t VMEM;
extern uint8_t font8x8_basic[128][8];
extern uint32_t _binary_data_bk_dat_start;
extern uint32_t _binary_data_it_dat_start;

#define CHAR_W 8
#define CHAR_H 8
#define SCR_W 640
#define SCR_H 480
#define SCR_SIZE (SCR_W * SCR_H)
#define SCR_BITSIZE (3 * SCR_W * SCR_H)

#define NR_SURFACE 50
#define POOL_SIZE (SCR_W * SCR_H * 3 * 5)

static int pe = 0, pl = 0;
static SURFACE surface[NR_SURFACE];
static uint8_t pool[POOL_SIZE];

static SURFACE * canvas;

void flip(SURFACE *src)
{
	int i;
	uint8_t *vmem = (uint8_t *)VMEM;
	uint8_t *pixels = src->pixels;
	if(src->is_video == false || src->pixels == NULL)
		return;
	for(i = 0; i < SCR_SIZE; i++)
	{
		vmem[0] = pixels[0];
		vmem[1] = pixels[1];
		vmem[2] = pixels[2];
		vmem += 3;
		pixels += 3;
	}
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

	int SrcX, SrcY, CopyWidth, CopyHeight, DstX, DstY;
	if(srcrect == NULL || dstrect == NULL)
	{
		SrcX = SrcY = DstX = DstY = 0;
		CopyWidth = src->w < dst->w ? src->w : dst->w;
		CopyHeight = src->h < dst->h ? src->h : dst->h;
	}
	else
	{
		int SrcCopyWidth = src->w - srcrect->x;
		int SrcCopyHeight = src->h - srcrect->y;
		int DstCopyWidth = dst->w - dstrect->x;
		int DstCopyHeight = dst->h - dstrect->y;
		int MinSDWidth = SrcCopyWidth < DstCopyWidth ? SrcCopyWidth : DstCopyWidth;
		int MinSDHeight = SrcCopyHeight < DstCopyHeight ? SrcCopyHeight : DstCopyHeight;
		SrcX = srcrect -> x;
		SrcY = srcrect -> y;
		DstX = dstrect -> x;
		DstY = dstrect -> y;
		CopyWidth = srcrect->w < MinSDWidth ? srcrect->w : MinSDWidth;
		CopyHeight = srcrect->h < MinSDHeight ? srcrect->h : MinSDHeight;
	}
	
	int i,j;
	uint8_t * SrcPos = src->pixels + 3 * SrcX + src->bpl * (0 + SrcY);
	uint8_t * DstPos = dst->pixels + 3 * DstX + dst->bpl * (0 + DstY);
	for(j = 0; j < CopyHeight; j++)
	{
		for(i = 0; i < CopyWidth; i++)
		{
			DstPos[0] = SrcPos[0];
			DstPos[1] = SrcPos[1];
			DstPos[2] = SrcPos[2];
			SrcPos += 3;
			DstPos += 3;
		}
		SrcPos = SrcPos - 3 * CopyWidth + src->bpl;
		DstPos = DstPos - 3 * CopyWidth + dst->bpl;
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

void font_out(int x, int y, uint32_t color, uint8_t times, const char *ctl, ...)
{
	char str[50];
	sprintk(str, ctl);
	int i, len = strlen(str);
	for(i = 0; i < len; i++)
	{
		font_sin(str[i], x + i * times * CHAR_H, y, color, times);
	}
}
