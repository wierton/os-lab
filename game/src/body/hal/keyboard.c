#include "body/game-common.h"

static uint32_t key_code = 0;

void keyboard_event(uint32_t code)
{
	key_code = code;
}

uint32_t query_keycode()
{
	return key_code & 0x7f;
}

uint32_t query_keystate()
{
	return (key_code >> 7) & 1;
}
