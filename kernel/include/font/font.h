#ifndef __FONT_H__
#define __FONT_H__

void draw_character(char ch, int x, int y, uint32_t color, uint8_t times);
void draw_str(int, int, uint32_t, uint8_t, const char *, ...);

#endif
