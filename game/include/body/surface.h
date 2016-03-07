#ifndef __SURFACE_H__
#define __SURFACE_H__

#define IDB_IT 0
#define IDB_BK 1

typedef struct {
	int x, y;
	uint32_t w, h;
} RECT;

typedef struct {
	int w, h;
	int bpl;//bytes per line
	uint32_t keycolor;
	bool is_video;
	void *pixels;
} SURFACE;

void flip(SURFACE *src);
SURFACE *create_surface(uint32_t w, uint32_t h);
void set_colorkey(SURFACE *src, uint32_t colorkey);
SURFACE * load_bmp(int ID);
void blit_surface(SURFACE *src, RECT *srcrect, SURFACE *dst, RECT*dstrect);
SURFACE * set_videomode();
void font_out(int x, int y, uint32_t color, uint8_t times, const char *ctl, ...);

#endif
