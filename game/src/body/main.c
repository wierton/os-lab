#include "body/game-common.h"

void init_res();
uint32_t get_ms();
void game_start();

SURFACE *screen, *background;

int game_init()
{
    srand(get_ms());
    
    screen = set_videomode();
        
    init_res();

	game_start();

	/* Should not reach here! */
	assert(0);
	return 0;
}
