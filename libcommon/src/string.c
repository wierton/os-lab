#include "common.h"
#include "string.h"

void inline memcpy(void *dst, size_t len, void *src)
{
	asm volatile("rep movsb"::"c"(len), "S"(src), "D"(dst));
}

void inline memset(void *dst, size_t len, uint8_t val)
{
	asm volatile("rep stosb"::"a"(val), "D"(dst));
}

int strcmp(char *src, char *dst)
{
	while(*src++ == *dst++);
	return *dst - *src;
}

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
