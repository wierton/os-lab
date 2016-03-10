#ifndef __LIB_H__
#define __LIB_H__

void printk(const char *ctl, ...);
void sprintk(char *dst, const char *ctl, ...);
void vsprintk(char *dst, const char *ctl, void **args);

void srand(int seed);
int rand();

int abs(int x);

float sin(float x);
float cos(float x);
float fabs(float x);
float sqrt(float x);

size_t strlen(char *str);

int strcat(char *, char *);

void abort(const char *, int);

#define assert(cond) \
	do { \
		if(!(cond)) \
		{ \
			abort(__FILE__, __LINE__);\
		} \
	} while(0)

#endif
