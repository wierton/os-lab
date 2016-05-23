#include <stdio.h>
#include <assert.h>
#include <malloc.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "common.h"
#include "fs.h"
	
char buf[1024 * 1024];
ROOT_INODE inode[INODE_SZ / sizeof(ROOT_INODE)];
FCB fcb[64];
int pfname = 0;
char filename[1024];

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
	FILE *fp = fopen(args[1], "w+");
	if(fp == NULL)
	{
		printf("Unknown error '%s'!\n", args[1]);
		return 0;
	}

	printf(":%d:\n", apply_block());
	printf(":%d:\n", apply_block());
	printf(":%d:\n", apply_block());
	free_block(0);
	printf(":%d:\n", apply_block());
	printf(":%d:\n", apply_block());
	free_block(1);

	/* write bootmgr */
	read_file(args[2], buf);
	fwrite(buf, BOOTMGR_SZ, 1, fp);

	/* write bitmap */
	memset(buf, 0xffffffff, 512 * 1024);
	fwrite(buf, BITMAP_SZ, 1, fp);

	/* write inode */
	memset(inode, 0xffffffff, sizeof(inode));
	fwrite(inode, INODE_SZ, 1, fp);

	/* constrct inode for file */
	inode[0].nr_block[0] = 0;
	int nr_block = 1, rinode, nr_inode = 1;
	for(i = 3; i < argv; i++)
	{
		printf("%s|%s\n", args[i], get_filename(args[i]));


		int filesz = get_filesz(args[i]);
		printf("blocks:%d\n", (filesz - 1) / BLOCKSZ);

		rinode = nr_inode;
		/* update fcb in root dir */
		fcb[i - 3].nr_root_inode = nr_inode ++;
		fcb[i - 3].filesz = filesz;
		fcb[i - 3].filetype = '-';
		fcb[i - 3].used = 1;
		fcb[i - 3].filename = sizeof(DIR_ATTR) + sizeof(FCB) * (argv - 3) + pfname;
		strcpy(&filename[pfname], get_filename(args[i]));
		pfname += strlen(get_filename(args[i])) + 1;

		if(filesz == 0) filesz = 1;
		for(j = 0; j < min(12, (filesz - 1) / BLOCKSZ + 1); j++)
		{
			inode[rinode].nr_block[j] = nr_block ++;
		}

		if(filesz > L0_SZ)
		{
			filesz -= L0_SZ;
			inode[rinode].index_1 = nr_inode;
			INDEX_L0 *index_l0 = (INDEX_L0 *)&inode[nr_inode ++];
			for(j = 0; j < min(16, (filesz - 1) / BLOCKSZ + 1); j++)
			{
				index_l0[0].nr_block[j] = nr_block ++;
			}
		}

		if(filesz > L1_SZ)
		{
			filesz -= L1_SZ;
			inode[rinode].index_2 = nr_inode;
			INDEX_L1 *index_l1 = (INDEX_L1 *)&inode[nr_inode ++];
			for(j = 0; j < min(16, (filesz - 1) / (16 * BLOCKSZ) + 1); j++)
			{
				index_l1[0].nr_index_l0[j] = nr_inode;
				INDEX_L0 *index_l0 = (INDEX_L0 *)&inode[nr_inode ++];
				for(k = 0; k < min(16, (filesz - j * 16 * BLOCKSZ - 1) / BLOCKSZ + 1); k++)
				{
					index_l0[0].nr_block[k] = nr_block ++;
				}
			}
		}
		
		if(filesz > L2_SZ)
		{
			filesz -= L2_SZ;
			inode[rinode].index_3 = nr_inode;
			INDEX_L2 *index_l2 = (INDEX_L2 *)&inode[nr_inode ++];
			for(j = 0; j < min(16, (filesz - 1) / (16 * 16 * BLOCKSZ) + 1); j++)
			{
				index_l2[0].nr_index_l1[j] = nr_inode;
				INDEX_L1 *index_l1 = (INDEX_L1 *)&inode[nr_inode ++];
				for(k = 0; k < min(16, (filesz - j * 16 * 16 * BLOCKSZ - 1) / (16 * BLOCKSZ) + 1); k++)
				{
					index_l1[0].nr_index_l0[k] = nr_inode ++;
					INDEX_L0 *index_l0 = (INDEX_L0 *)&inode[nr_inode ++];
					for(l = 0; l < min(16, (filesz - j * 16 * 16 * BLOCKSZ - k * 16 * BLOCKSZ - 1) / BLOCKSZ + 1); l++)
					{
						index_l0[0].nr_block[l] = nr_block ++;
					}
				}
			}
		}

		if(filesz > L3_SZ)
			assert(0);
	}

	for(i = 0; i < pfname; i++)
		printf("%c", filename[i] == 0 ? ' ': filename[i]);
	printf("\n");

	/* update inode */
	fseek(fp, INODE_ST, SEEK_SET);
	fwrite(inode, INODE_SZ, 1, fp);
	
	/* the first block is root directory file */
	DIR_ATTR dir_attr;
	dir_attr.nr_files = argv - 3;
	dir_attr.filename_st = sizeof(DIR_ATTR) + sizeof(FCB) * (argv - 3);
	dir_attr.filesz = sizeof(DIR_ATTR) + sizeof(FCB) * (argv - 3) + pfname;
	memcpy(buf, &dir_attr, sizeof(DIR_ATTR));
	memcpy(buf + sizeof(DIR_ATTR), fcb, sizeof(FCB) * (argv - 3));
	memcpy(buf + sizeof(DIR_ATTR) + sizeof(FCB) * (argv - 3), filename, pfname);
	fwrite(buf, BLOCKSZ, 1, fp);

	printf("%d, %d, %d\n", sizeof(DIR_ATTR), sizeof(FCB) * (argv - 3), pfname);

	/* write file */
	int total_filesz = 0;
	for(i = 3; i < argv; i++)
	{
		int filesz = get_filesz(args[i]);
		total_filesz += (filesz / BLOCKSZ + 1) * BLOCKSZ;
		assert(filesz < sizeof(buf));
		read_file(args[i], buf);
		fwrite(buf, (filesz / BLOCKSZ + 1) * BLOCKSZ, 1, fp);
		printf("filesz:%d, %d, %d\n", filesz, (filesz / BLOCKSZ + 1) * BLOCKSZ, get_filesz(args[1]));
	}

	/* update bitmap */
	uint32_t bits = total_filesz / BLOCKSZ;
	assert(bits > 1);
	fseek(fp, BITMAP_ST, SEEK_SET);
	fread(buf, BITMAP_SZ, 1, fp);
	for(i = 0; i < (bits - 1) / 8; i++)
	{
		buf[i] = 0;
	}
	buf[i] = ((1 << (8 - (bits % 8))) - 1);
	fseek(fp, BITMAP_ST, SEEK_SET);
	fwrite(buf, BITMAP_SZ, 1, fp);
	//printf("|%d, %d|, |%d, %d|, %x, %hhx\n", total_filesz, BLOCKSZ, bits, i, bits % 8, buf[i]);
	printf("%d, %d, %d, %d\n", min(3, 5), max(3, 5), min(4, 4), max(4, 4));

	fclose(fp);

	return 0;
}
