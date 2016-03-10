#include "body/game-common.h"

static uint32_t key_code, trav = 0;

void keyboard_event(uint32_t code)
{
	if(key_code == 0xe0)
	{
		key_code = ((key_code << 8) | code);
	}
	else
	{
		key_code = code;
	}
	trav = 0;
}

uint32_t read_key()
{
	if(trav == 0)
	{
		trav = 1;
		return key_code & 0xffff;
	}
	else
	{
		return 0;
	}
}

