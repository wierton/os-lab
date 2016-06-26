#include "common.h"
#include "x86/x86.h"

#define INVALID_SCANCODE 0xffffffff

static int shift = 0;
static uint32_t key_code, trav = 0;
char switch_to_ascii(uint32_t code);

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
	void update_buf(int ch);
	void write_char(char ch);
	void call_history(int ch);
	if(key_code != 0xe0 && ((key_code & 0x80) == 0))
	{
		write_char(switch_to_ascii(key_code));
		update_buf(switch_to_ascii(key_code));
		//printk("%x\t", key_code);
	}
	if((key_code >> 8) == 0xe0)
		call_history(key_code);
	if(key_code == 0x2a)
		shift = 1;
	if(key_code == (0x2a | 0x80))
		shift = 0;
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

char read_ascii()
{
	return switch_to_ascii(key_code);
}

char switch_to_ascii(uint32_t code)
{
	switch(code & 0xff7f)
	{
		case 0x1c: return '\n';
		case 0x39: return ' ';
		case 0x0e: return '\b';
		case 0x1a: return shift ? '{' : '[';
		case 0x1b: return shift ? '}' : ']';
		case 0x27: return shift ? ':' : ';';
		case 0x28: return shift ? '"' : '\'';
		case 0x33: return shift ? '<' : ',';
		case 0x34: return shift ? '>' : '.';
		case 0x35: return shift ? '?' : '/';
		case 0x2b: return shift ? '|' : '\\';
		case 0x0c: return '-';
		case 0x1e: return 'a';
		case 0x30: return 'b';
		case 0x2e: return 'c';
		case 0x20: return 'd';
		case 0x12: return 'e';
		case 0x21: return 'f';
		case 0x22: return 'g';
		case 0x23: return 'h';
		case 0x17: return 'i';
		case 0x24: return 'j';
		case 0x25: return 'k';
		case 0x26: return 'l';
		case 0x32: return 'm';
		case 0x31: return 'n';
		case 0x18: return 'o';
		case 0x19: return 'p';
		case 0x10: return 'q';
		case 0x13: return 'r';
		case 0x1f: return 's';
		case 0x14: return 't';
		case 0x16: return 'u';
		case 0x2f: return 'v';
		case 0x11: return 'w';
		case 0x2d: return 'x';
		case 0x15: return 'y';
		case 0x2c: return 'z';
		case 0x0b: return '0';
		case 0x02: return '1';
		case 0x03: return '2';
		case 0x04: return '3';
		case 0x05: return '4';
		case 0x06: return '5';
		case 0x07: return '6';
		case 0x08: return '7';
		case 0x09: return '8';
		case 0x0a: return '9';
	}
	return 0;
}

void init_keyboard()
{
	//TODO: init the keyboard
}
