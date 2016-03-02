#ifndef __VIDEO_H__
#define __VIDEO_H__

extern uint32_t _binary_data_bk_dat_start; //the dat

struct ModeInfoBlock {
	uint16_t attributes;
	uint8_t winA,winB;
	uint16_t granularity;
	uint16_t winsize;
	uint16_t segmentA, segmentB;
	uint32_t undefined;
	//VBE_FAR(realFctPtr);
	uint16_t pitch; //bytes per scanline
	
	uint16_t Xres, Yres;
	uint8_t Wchar, Ychar, planes, bpp, banks;
	uint8_t memory_model, bank_size, image_pages;
	uint8_t reserved0;
	
	uint8_t red_mask, red_position;
	uint8_t green_mask, green_position;
	uint8_t blue_mask, blue_position;
	uint8_t rsv_mask, rsv_position;
	uint8_t directcolor_attributes;
	
	uint32_t physbase;  //your LFB (Linear Framebuffer) address ;)
	uint32_t reserved1;
	uint16_t reserved2;
	} __attribute__((packed));

typedef struct tagBITMAPFILEHEADER {
	WORD  bfType;
	DWORD bfSize;
	WORD  bfReserved1;
	WORD  bfReserved2;
	DWORD bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
	DWORD biSize;
	LONG  biWidth;
	LONG  biHeight;
	WORD  biPlanes;
	WORD  biBitCount;
	DWORD biCompression;
	DWORD biSizeImage;
	LONG  biXPelsPerMeter;
	LONG  biYPelsPerMeter;
	DWORD biClrUsed;
	DWORD biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;


typedef struct SDL_Surface {
	int w, h;
	void *pixels;
} SDL_Surface;

uint8_t get_r(uint32_t color);
uint8_t get_g(uint32_t color);
uint8_t get_b(uint32_t color);

uint32_t make_rgb(uint8_t r, uint8_t g, uint8_t b);

void draw_rect(int x, int y, int w, int h, uint32_t color);

void draw_surface();

#endif
