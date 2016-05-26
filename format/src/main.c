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
char buf[4 * 1024 * 1024];
char tbuf[4 * 1024 * 1024];
char rootdir[4 * 1024];
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

uint32_t read_offfile(char *filename, int op, int ed, char *buf)
{
	FILE *fp = fopen(filename, "r");
	fseek(fp, op, SEEK_SET);
	fread(buf, ed - op + 1, 1, fp);
	fclose(fp);
	return ed - op + 1;
}

uint32_t write_file(char *filename, char *buf, int size)
{
	FILE *fp = fopen(filename, "w+");
	fwrite(buf, size, 1, fp);
	fclose(fp);
	return size;
}

/* args: bootmgr, file1, file2, file3 ...
 * */
int main(int argv, char *args[])
{
	int i, j;
	fp = fopen(args[1], "w+");
	if(fp == NULL)
	{
		printf("Unknown error '%s'!\n", args[1]);
		return 0;
	}

	init_disk(argv, args);

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
	for(i = 0; i < 4; i++)
	{
		memset(buf, 0, 512 * 1024);
		fwrite(buf, 512 * 1024, 1, fp);
	}

	/* write file */
	for(i = 3; i < argv; i++)
	{
		char str[200];
		int filesz = get_filesz(args[i]);
		INODE *pinode = &inode[apply_inode()];
		pinode->used = 1;
		pinode->filesz = 0;
		assert(filesz < sizeof(buf));
		read_file(args[i], buf);
		fs_write(pinode, 0, filesz, buf);
		/* test code for fs_write and fs_read */
		memset(tbuf, 0, sizeof(tbuf));
		fs_read(pinode, 0, filesz, tbuf);
		for(j = 0; j < filesz; j++)
			assert(buf[j] == tbuf[j]);
		printf("%d %d %d\n", filesz, tbuf[j - 1], tbuf[j]);
	}

	/* test fs_read and fs_write for unaligned read and write operation */
/*
	INODE *pinode = &inode[apply_inode()];
	int filesz = get_filesz(args[3]);
	pinode->used = 1;
	pinode->filesz = 0;
	for(i = 0; i < 12; i++)
	{
		read_offfile(args[3], filesz*i/12, filesz*(i+1)/12, buf);
		fs_write(pinode, filesz*i/12, filesz*(i+1)/12, buf);
		fs_read(pinode, filesz*i/12, filesz*(i+1)/12, tbuf + filesz*i/12);

		printf("[%d, %d) ", filesz*i/12, filesz*(i+1)/12);
	}
	printf("\n");
	write_file("obj/tmp", tbuf, filesz);
*/
	/* update root directory file */

	/* update bitmap */
	fseek(fp, BITMAP_ST, SEEK_SET);
	fwrite(bitmap, BITMAP_SZ, 1, fp);

	/* write inode */
	fseek(fp, INODE_ST, SEEK_SET);
	fwrite(inode, INODE_SZ, 1, fp);

	fclose(fp);

	return 0;
}
