#include "common.h"
#include "string.h"
#include "x86/x86.h"
#include "device/disk.h"
#include "device/fs.h"

#define NR_FCB 128

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define MODE_R (1 << 0)
#define MODE_W (1 << 1)
#define MODE_G (1 << 2)

typedef struct {
	int dirty, mode;
	uint32_t offset;
   	INODE *pinode;
} FCB;

FCB fcb[128];

void init_ufs()
{
	int i;
	for(i = 0; i < NR_FCB; i++)
	{
		fcb[i].dirty = 0;
		fcb[i].offset = 0;
		fcb[i].pinode = NULL;
	}
}

/* int open(const char *pathname, int flags, mode_t mode); */
int open(TrapFrame *tf)
{
	int i, ret = opendir((void *)(tf->ebx));
	if(ret == INVALID_INODENO)
	{
		if((tf->edx & MODE_G) != 0)
		{
			ret = creat((void *)(tf->ebx));
		}
		else
			return -1;
	}

	for(i = 0; i < NR_FCB; i++)
	{
		if(fcb[i].dirty == 0)
		{
			fcb[i].dirty = 1;
			fcb[i].mode = tf->edx;
			fcb[i].offset = 0;
			fcb[i].pinode = open_inode(ret);
			if(fcb[i].pinode->filetype == 'd')
				return -1;
			return i;
		}
	}
	return -1;
}

/* int close(int fd); */
int close(TrapFrame *tf)
{
	uint32_t fd = tf->ebx;
	if(fd >= NR_FCB)
		return -1;
	fcb[fd].dirty = 0;
	close_inode(fcb[fd].pinode);
	return 0;
}

/* size_t read(int fd, void *buf, size_t count) */
int read(TrapFrame *tf)
{
	uint32_t fd = tf->ebx;
	if(fd >= NR_FCB || (fcb[fd].mode & MODE_R) == 0 || fcb[fd].dirty == 0)
		return -1;

	int size = fs_read(fcb[fd].pinode, fcb[fd].offset, tf->edx, (void *)(tf->ecx));
	fcb[fd].offset += size;
	return size;
}

/* size_t write(int fd, const void *buf, size_t count) */
int write(TrapFrame *tf)
{
	uint32_t fd = tf->ebx;
	if(fd >= NR_FCB || (fcb[fd].mode & MODE_R) == 0 || fcb[fd].dirty == 0)
		return -1;

	int size = fs_write(fcb[fd].pinode, fcb[fd].offset, tf->edx, (void *)(tf->ecx));
	fcb[fd].offset += size;
	return size;
}

/*  off_t lseek(int fd, off_t offset, int whence); */
int lseek(TrapFrame *tf)
{
	uint32_t fd = tf->ebx;
	if(fd >= NR_FCB)
		return -1;

	switch(tf->edx)
	{
		case SEEK_SET:fcb[fd].offset = tf->ecx;break;
		case SEEK_CUR:fcb[fd].offset += tf->ecx;break;
		case SEEK_END:fcb[fd].offset = fcb[fd].pinode->filesz + tf->ecx;break;
		default: break;
	}
	return fcb[fd].offset;
}
