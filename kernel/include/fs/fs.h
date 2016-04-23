#ifndef __FS_H__
#define __FS_H__


typedef struct {
	uint8_t attr;
	char name[32];
	uint32_t size, offset;
} fs_block;



#endif
