#include "body/game-common.h"

static uint32_t jiffy = 0;

void timer_event()
{
	jiffy ++;
}

void delay(uint32_t ms)
{
	uint32_t dst = jiffy + ms;
	while(jiffy < dst);
}

uint32_t get_ms()
{
	return jiffy;
}
