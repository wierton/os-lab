#include "common.h"
#include "video.h"

#define BK_W 800
#define BK_H 600
uint32_t VMEM;

void init_video()
{
	struct ModeInfoBlock *it = (struct ModeInfoBlock *)0x2000;
	VMEM = it->physbase;
//	draw_rect(-100, -100, 800, 800, make_rgb(0x0, 0, 0xff));
	draw_surface();
}

uint8_t get_r(uint32_t color)
{
	return ((color >> 16) & 0xff);
}

uint8_t get_g(uint32_t color)
{
	return ((color >> 8) & 0xff);
}

uint8_t get_b(uint32_t color)
{
	return ((color >> 0) & 0xff);
}

uint32_t make_rgb(uint8_t r, uint8_t g, uint8_t b)
{
	return ((r << 16) | (g << 8) | b);
}

void draw_rect(int x, int y, int w, int h, uint32_t color)
{
	/*clip the area firstly to make it safe to video memory
	 */
	if(x >= BK_W || y >= BK_H || (x + w) < 0 || (y + h) < 0)
		return;

	/*caculate the intersection area of two rectangle
	 */
	int16_t opx = max(0, x);
	int16_t opy = max(0, y);
	int16_t edx = min((int)BK_W, x + w);
	int16_t edy = min((int)BK_H, y + h);
	int16_t nw = 3 * (edx - opx);
	int i, j;
	uint32_t bytes = ((3 * BK_W + 3) & ~3);
	uint8_t *vmem = (uint8_t *)VMEM + 3 * opx + opy * bytes;
	for(j = opy; j < edy; j++)
	{
		for(i = opx; i < edx; i++)
		{
			((uint32_t *)vmem)[0] = color;
				vmem += 3;
		}
		vmem = vmem - nw + bytes;
	}
}

extern uint32_t _binary_data_bk_dat_size;

void draw_surface()
{
	int i, j;
	uint32_t *bk = (uint32_t *)(&_binary_data_bk_dat_start);
	printk("size:%d\n", &_binary_data_bk_dat_size);
	PBITMAPINFOHEADER bi = (PBITMAPINFOHEADER)((void *)bk + 14);
	uint8_t * pixels = (void *)bk + 54;
	printk("0x%x,%d,%d\n", VMEM, bi->biWidth, bi->biHeight);
	draw_rect(0, 0, 800, 600, make_rgb(0x12, 0x45, 0x90));
	for(j = 0; j < 600; j++)
	{
		for(i = 0; i < 800; i++)
		{
			uint8_t * vmem = (void *)(VMEM + 3 * i + j * 2400);
			uint8_t * src = (void *)(pixels + 3 * i + j * 2400);
			vmem[0] = src[0];
			vmem[1] = src[1];
			vmem[2] = src[2];
		}
	}
	printk("****:%d\n", ((uint32_t *)0x8000)[0]);
	
}
