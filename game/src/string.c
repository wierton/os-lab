#include "common.h"
#include "lib.h"

size_t strlen(char *str)
{
	char *ptr = str;
	while(*ptr++);
	return ((size_t)(ptr - str));
}
