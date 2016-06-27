#include "common.h"
#include "draw.h"
#include "lib.h"

int main()
{
	/*
	int w = 10, h = 10;
	int x = SCR_W / 2, y = h;
	int cx = 2, cy = 2;
	while(1)
	{
		draw_rect(0x0000ff, 0, 0, SCR_W, SCR_H);
		draw_rect(0, x += cx, y += cy, w, h);
		if(x > SCR_W || x <= 0)
			cx = - cx;
		if(y > SCR_H || y <= 0)
			cy = - cy;
		invalidate();
	}
	*/
	while(1) {
		int ret = -1;
		ret = syscall(0x1b);
		//printf("%d, ", ret);
		if(ret == 1)
			asm volatile("movl $0x1a,%eax; int $0x80");
		else if(ret == 0)
		{
			printf("idle has been started!\n");
			break;
		}
	}
	return 0;
}
