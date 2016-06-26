#include "common.h"
#include "string.h"

void inline memcpy(void *dst, void *src, size_t len)
{
	asm volatile("cld; rep movsb"::"c"(len), "S"(src), "D"(dst));
}

void inline memset(void *dst, uint32_t val, size_t len)
{
	int i;
	for(i = 0; i < len; i++)
		((uint8_t *)dst)[i] = val;
//	asm volatile("rep stosb"::"a"(val), "c"(len), "D"(dst));
}

int strcmp(char *src, char *dst)
{
	while(src[0] && dst[0] && src[0] == dst[0])
	{
		src ++;	dst ++;
	}
	return src[0] - dst[0];
}

size_t strlen(char *str)
{
	char *ptr = str;
	while(*ptr++);
	return ((size_t)(ptr - str - 1));
}

size_t strcpy(char *dtr, char *str)
{
	char *ptr = str;
	while(*ptr) {*dtr ++ = *ptr ++;};
	*dtr = 0;
	return ((size_t)(ptr - str - 1));
}

int strcat(char *dst, char *src)
{
	int i, dlen = strlen(dst), slen = strlen(src);
	for(i = 0; i <= slen; i++)
		dst[i + dlen] = src[i];
	dst[i + dlen] = 0;
	return 0;
}

char *strtok(char *str, char ch)
{
	char *ptr = str;
	if(str == NULL)
		return NULL;
	while(*ptr && *ptr != ch) {ptr ++;};
	if(*ptr == 0)
		return str;
	while(*ptr && *ptr == ch) {*ptr = 0; ptr ++;};
	if(*ptr == 0)
		return NULL;
	*(ptr - 1) = 0;
	return ptr;
}
