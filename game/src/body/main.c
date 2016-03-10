#include "body/game-common.h"

void init_res();
void game_start();

SURFACE *screen, *background;

int game_init()
{
    srand(600);
    
    screen = set_videomode();
        
    init_res();

	game_start();

	/* Should not reach here! */
	assert(0);
	return 0;
}
