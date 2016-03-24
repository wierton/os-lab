#include "common.h"

void move_player();
uint32_t read_key();

INPUTSTATE g_InputState = {kdirUnknown, kdirUnknown, KEY_UNKNOWN};

void msgloop()
{
	uint32_t key = read_key();
	uint32_t keystate = (key >> 7 ) & 1;
	uint32_t keycode = key & 0xff7f;

	if(key != INVALID_SCANCODE && keystate == OS_KEYDOWN)
	{
		switch(keycode)
		{
			//dir key
			case KEY_UP	:g_InputState.dir = kdirUp;	break;
			case KEY_DOWN	:g_InputState.dir = kdirDown;	break;
			case KEY_LEFT	:g_InputState.dir = kdirLeft;	break;
			case KEY_RIGHT	:g_InputState.dir = kdirRight;	break;
			default:
				g_InputState.dwKey = keycode;
				break;
		}
	}
	else if(key != INVALID_SCANCODE && keystate == OS_KEYUP)
	{
		switch(keycode)
		{
			case KEY_UP	:g_InputState.dir = kdirUnknown;
							 break;
			case KEY_DOWN	:g_InputState.dir = kdirUnknown;
							 break;
			case KEY_LEFT	:g_InputState.dir = kdirUnknown;
							 break;
			case KEY_RIGHT	:g_InputState.dir = kdirUnknown;
							 break;
			default:
				g_InputState.dwKey = KEY_UNKNOWN;
				break;
		}
	}
	move_player();
}
