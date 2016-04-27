#include "common.h"
#include "x86/x86.h"

#define PORT_CH_0 0x40
#define PORT_CMD 0x43
#define PIT_FREQUENCE 1193182
#define HZ 100

static uint32_t jiffy = 0;

void timer_event(TrapFrame *tf)
{
	jiffy ++;
}

int time()
{
	return jiffy;
}

void init_timer()
{
	int counter = PIT_FREQUENCE / HZ;
	assert(counter < 65536);
	out_byte(PORT_CMD, 0x34);
	out_byte(PORT_CH_0, counter & 0xFF);         // access low byte
	out_byte(PORT_CH_0, (counter >> 8) & 0xFF);  // access high byte
}

