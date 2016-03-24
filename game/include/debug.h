#ifndef __DEBUG_H__
#define __DEBUG_H__

#define DEBUG

#define Log(format, ...) \
	do { \
		printf(stdout, "\33[1;34m[%s,%d,%s] " format "\33[0m\n", \
				__FILE__, __LINE__, __func__, ## __VA_ARGS__); \
	} while(0)

#define Assert(cond, ...) \
	do { \
		if(!(cond)) { \
			printf("\33[1;31m"); \
			printf(__VA_ARGS__); \
			printf("\33[0m\n"); \
			assert(cond); \
		} \
	} while(0)

#define panic(format, ...) \
	Assert(0, format, ## __VA_ARGS__)

#endif
