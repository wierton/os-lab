#ifndef __STRING_H__
#define __STRING_H__

void inline memcpy(void *dst, void *src, size_t len);
void inline memset(void *dst, uint32_t val, size_t len);
size_t strlen(char *);
size_t strcpy(char *dtr, char *str);
int strcat(char *, char *);
int strcmp(char *dst, char *src);
char *strtok(char *str, char ch);

#endif
