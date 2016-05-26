#include <stdio.h>
#include <assert.h>
#include <malloc.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "common.h"
#include "fs.h"

FILE *fp;
char buf[1024 * 1024];
int pfname = 0;
char filename[1024];

extern uint32_t bitmap[512 * 1024 / 32];
extern INODE inode[512 * 1024 / sizeof(INODE)];

char *get_filename(char *fullpath)
{
	int i = 0, j = 0;
	while(fullpath[j])
	{
		if(fullpath[j ++] == '/')
			i = j;
	}
	return &fullpath[i];
}

int get_filesz(char *filename)  
{  
	struct stat buf;  
	if(stat(filename, &buf)<0)  
	{  
	    return 0;  
	}  
	return buf.st_size;  
}

uint32_t read_file(char *filename, char *buf)
{
	int filesz = get_filesz(filename);
	FILE *fp = fopen(filename, "r");
	fread(buf, filesz, 1, fp);
	fclose(fp);
	return filesz;
}

uint32_t apply_block();
void free_block(uint32_t);

/* args: bootmgr, file1, file2, file3 ...
 * */
int main(int argv, char *args[])
{
	int i, j, k, l;
	fp = fopen(args[1], "w+");
	printf("%s\n", args[1]);
	if(fp == NULL)
	{
		printf("Unknown error '%s'!\n", args[1]);
		return 0;
	}

	init_disk(argv, args);

	printf("%d, %d, %d\n", BOOTMGR_SZ, BITMAP_SZ, INODE_SZ);
	/* write bootmgr */
	read_file(args[2], buf);
	fwrite(buf, BOOTMGR_SZ, 1, fp);

	/* write bitmap */
	memset(buf, 0, 512 * 1024);
	fwrite(buf, BITMAP_SZ, 1, fp);

	/* write inode */
	memset(buf, 0, INODE_SZ);
	fwrite(buf, INODE_SZ, 1, fp);

	/* empty space for file */
	for(i = 0; i < 10; i++)
	{
		memset(buf, 0, 512 * 1024);
		fwrite(buf, 512 * 1024, 1, fp);
	}

	INODE testinode = {0};
	printf("%x, %x\n", INVALID_BLOCKNO, INVALID_INODENO);

	/* write file */
	for(i = 3; i < argv; i++)
	{
		int filesz = get_filesz(args[i]);
		INODE inode = {0};
		inode.used = 1;
		inode.filesz = 0;
		assert(filesz < sizeof(buf));
		read_file(args[i], buf);
		fs_write(&inode, 0, filesz, buf);
		printf("filesz %s:%d, %d, %d\n\n", args[i], filesz, (filesz / BLOCKSZ + 1) * BLOCKSZ, get_filesz(args[1]));
	}

	fclose(fp);

	return 0;
}
