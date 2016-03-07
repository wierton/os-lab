#ifndef __DEBUG_H__
#define __DEBUG_H__

#define DEBUG

#define Log(format, ...) \
	do { \
		printk(stdout, "\33[1;34m[%s,%d,%s] " format "\33[0m\n", \
				__FILE__, __LINE__, __func__, ## __VA_ARGS__); \
	} while(0)

#define Assert(cond, ...) \
	do { \
		if(!(cond)) { \
			printk("\33[1;31m"); \
			printk(__VA_ARGS__); \
			printk("\33[0m\n"); \
			assert(cond); \
		} \
	} while(0)

#define panic(format, ...) \
	Assert(0, format, ## __VA_ARGS__)

#endif
