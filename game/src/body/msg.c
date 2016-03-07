#include "body/game-common.h"

void move_player();
uint32_t query_keycode();
uint32_t query_keystate();

INPUTSTATE g_InputState = {kdirUnknown, kdirUnknown, KEY_Unknown};

void msgloop()
{
	uint32_t keystate = query_keystate();
	uint32_t keycode = query_keycode();

	if(keystate == OS_KEYDOWN)
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
	else if(keystate == OS_KEYUP)
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
				g_InputState.dwKey = KEY_Unknown;
				break;
		}
	}
	move_player();
}
