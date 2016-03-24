#include "common.h"
#include "x86/x86.h"

#define INVALID_SCANCODE 0xffffffff

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

int read_key(TrapFrame *tf)
{
	if(trav == 0)
	{
		trav = 1;
		return key_code & 0xffff;
	}
	else
	{
		return INVALID_SCANCODE;
	}
}

void init_keyboard()
{
	//TODO: init the keyboard
}
