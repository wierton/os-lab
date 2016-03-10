#include "common.h"
#include "lib.h"
/*
void memcpy_asm(void *dst, size_t len, void *src)
{
	asm volatile("cld;\
			movl %0, %%ecx;\
			movl %1, %%esi;\
			movl %2, %%edi;\
			rep movsl;
			"::"m"(len), "a"(src), "b"(dst));
}
*/
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
