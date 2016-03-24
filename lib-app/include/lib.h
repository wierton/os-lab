#ifndef __LIB_H__
#define __LIB_H__

void printf(const char *ctl, ...);
void sprintf(char *dst, const char *ctl, ...);
void vsprintf(char *dst, const char *ctl, void **args);

void srand(int seed);
int rand();

void abort(const char *, int);

int __attribute__((noinline)) syscall(int ID, ...);

#define assert(cond) \
	do { \
		if(!(cond)) \
		{ \
			abort(__FILE__, __LINE__); \
		} \
	} while(0)

#endif
