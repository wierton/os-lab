#include "common.h"
#include "lib.h"

size_t strlen(char *str)
{
	char *ptr = str;
	while(*ptr++);
	return ((size_t)(ptr - str));
}

int strcat(char *dst, char *src)
{
	size_t len = strlen(dst);
	dst = dst + len;
	for(; *src != 0;)
	{
		*dst++ = *src++;
	}
	return 0;
}
