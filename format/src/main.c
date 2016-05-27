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
char buf[8 * 1024 * 1024];
char tbuf[8 * 1024 * 1024];
char rootdir[4 * 1024];
uint32_t bitmap[512 * 1024 / 32];
INODE inode[512 * 1024 / sizeof(INODE)];

char *get_filename(char *fullpath)
{
	int i = 0, j = 1;
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

uint32_t read_offfile(char *filename, int op, int size, char *buf)
{
	FILE *fp = fopen(filename, "r");
	fseek(fp, op, SEEK_SET);
	int tmp = fread(buf, size, 1, fp);
	fclose(fp);
	return tmp;
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
/*
	int tmp = 0xffffffff;
	for(i = 0; i < 8192; i++)
		((int*)buf)[i] = i;
	INODE *ptinode = &inode[0];
	fs_write(ptinode, 0, 8192, buf);
	for(i = 0; i < 8192; i += 4)
	{
		fs_read(ptinode, i, 4, &tmp);
		printf("%08x ", tmp);
	}
	printf("\n");
*/
	printf("macro: %x, %x, %d\n", INODE_ST, BITMAP_ST, sizeof(INODE));

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
		{
			assert(buf[j] == tbuf[j]);
		}
	}

/*
	memset(tbuf, 0xcc, sizeof(tbuf));
	fs_read(&inode[0], 2048, 4096, (void*)tbuf + 2048);
	printf("%x, %x, %x, %x\n", ((uint8_t*)tbuf)[2047], ((uint8_t*)tbuf)[2048], ((uint8_t*)tbuf)[2048 + 4095], ((uint8_t*)tbuf)[2048 + 4096]);
*/
	/* test fs_read and fs_write for unaligned read and write operation */
/*
	INODE *pinode = &inode[0];
	int filesz = get_filesz(args[3]);
	pinode->used = 1;
	pinode->filesz = 0;
	memset(tbuf, 0xcc, sizeof(tbuf));
	for(i = 0; i < 4; i++)
	{
		int diff = filesz*(i+1)/4 - filesz*i/4;
		memset(buf, 0xcc, sizeof(buf));
		read_offfile(args[3], filesz*i/4, diff, buf);
		fs_write(pinode, filesz*i/4, diff, buf);
		fs_read(pinode, filesz*i/4, diff, tbuf + filesz*i/4);
	}
	write_file("obj/tmp", tbuf, filesz);
*/
	/* update root directory file */
	inode[0].nr_block[0] = 0;
	for(i = 0; i < argv - 3; i++)
	{
		inode[i + 1].filetype = '-';
		addto_dir(&inode[0], &inode[i + 1], get_filename(args[i + 3]));
	}

	for(i = 0; i < argv - 3; i++)
	{
		char path[20];
		sprintf(path, "/%s", get_filename(args[i + 3]));
		printf("%s, %d\n", get_filename(args[i + 3]), opendir(path));
	}

	/* update bitmap */
	fseek(fp, BITMAP_ST, SEEK_SET);
	fwrite(bitmap, BITMAP_SZ, 1, fp);

	/* update inode */
	fseek(fp, INODE_ST, SEEK_SET);
	fwrite(inode, INODE_SZ, 1, fp);

	fclose(fp);

	return 0;
}
