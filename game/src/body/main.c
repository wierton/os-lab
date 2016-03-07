#include "body/game-common.h"

void init_res();
void init_enemy();
void init_bullet();
void init_player();
void init_plane();
void init_bomb();
void init_xbullets();
void game_start();

SURFACE *screen, *background;

int game_init()
{
    srand(600);
    
    screen = set_videomode();
        
    init_res();

	init_player();

	init_plane();

	init_bullet();

	init_bomb();
	
	init_xbullets();

	game_start();

	/* Should not reach here! */
	assert(0);
	return 0;
}
