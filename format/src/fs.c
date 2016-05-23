#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "common.h"
#include "fs.h"

/* l0 index: 0 - 6291455
 * l1 index: 6291456 - 14680063
 * l2 index: 14680064 - 148897791
 * l3 index: 148897792 - 229638144
 * l4:unused
 */

uint32_t bitmap[512 * 1024 / 32];

int read_disk(uint32_t buf, uint32_t off, uint32_t size)
{
	FILE *fp = fopen("disk.img", "r");
	fseek(fp, off, SEEK_SET);
	uint32_t ret = fread((void*)buf, size, 1, fp);
	fclose(fp);
	return ret;
}

inline int get_bit(uint32_t val, uint32_t pos)
{
	pos = pos & 0x1f;
	return (val >> pos) & 1;
}

inline uint32_t set_bit(uint32_t val, uint32_t pos, uint32_t bit)
{
	pos = pos & 0x1f;
	bit = (bit & 0x1) << pos;
	uint32_t mask = ~(1 << pos);
	return (val & mask) | bit;
}

void init_disk()
{
	memset(bitmap, 0, sizeof(bitmap));
}

uint32_t apply_block()
{
	int i, j;
	for(i = 0; i < sizeof(bitmap)/sizeof(uint32_t); i++)
	{
		if(bitmap[i] != 0xffffffff)
		{
			for(j = 31; j >= 0; j--)
			{
				if((1 & (bitmap[i] >> j)) == 0)
				{
					printf("%x %x ", bitmap[i], j);
					uint32_t mask = 1 << j;
					bitmap[i] |= mask;
					printf("%x\n", bitmap[i]);
					return i * 32 + (31 - j);
				}
			}
		}
	}
	assert(0);
	return 0xffffffff;
}

void free_block(uint32_t nr_block)
{
	assert(nr_block < sizeof(bitmap)/sizeof(uint32_t));
	printf("%x ", bitmap[nr_block / 32]);
	uint32_t mask = ~(1 << (31 - (nr_block % 32)));
	bitmap[nr_block / 32] &= mask;
	printf("%x %x\n", bitmap[nr_block / 32], mask);
}

int read_l0(ROOT_INODE *rp_inode, uint32_t offset, uint32_t size, uint8_t *buf)
{
	assert((offset + size) < L0_SZ);
	uint32_t st_block = rp_inode->nr_block[offset / L0_PSZ];
	uint32_t ed_block = rp_inode->nr_block[(offset + size) / L0_PSZ];
	/* read the first block */
	uint32_t st_off = FILE_ST + st_block * BLOCKSZ + offset % BLOCKSZ;
	uint32_t st_sz = BLOCKSZ - (offset % BLOCKSZ);
	read_disk((uint32_t)buf, st_off, st_sz);
	buf += st_sz;

	/* read the medium block */
	uint32_t i, moff = FILE_ST + (st_block + 1) * BLOCKSZ;
	for(i = offset / L0_PSZ + 1; i < (offset + size) / L0_PSZ; i++)
	{
		uint32_t nr_block = rp_inode->nr_block[i];
		read_disk((uint32_t)buf, moff, BLOCKSZ);
		buf += BLOCKSZ;
		moff += BLOCKSZ;
	}

	/* read the last block */
	uint32_t ed_off = FILE_ST + ed_block * BLOCKSZ;
	uint32_t ed_sz = ((offset + size) % BLOCKSZ);
	read_disk((uint32_t)buf, ed_off, ed_sz);

	return size;
}

int fs_read(FCB *fcb, uint32_t offset, uint32_t size, uint8_t *buf)
{
	assert(fcb->nr_root_inode < NR_INODE);
	ROOT_INODE root_inode;
	uint32_t fi_size = offset + size;
	uint32_t disk_off = INODE_ST + sizeof(ROOT_INODE) * fcb->nr_root_inode;
	read_disk(((uint32_t)&root_inode), disk_off, sizeof(ROOT_INODE));

	uint32_t rd_size = 0;
	if(offset < L0_SZ)
	{
		rd_size = min(offset + size, L0_SZ) - offset;
		read_l0(&root_inode, offset, rd_size, buf);
	}
	if(fi_size > L0_SZ)
	{
		if(offset < L1_SZ)
		{}
	}

	return size;
}

uint32_t opendir(uint32_t inode, char *dir)
{
	int i;
	DIR_ATTR dir_attr;
	FCB fcb[64];
	read_disk((uint32_t)&dir_attr, INODE_ST + inode * sizeof(DIR_ATTR), sizeof(DIR_ATTR));
	
	for(i = 0; i < dir_attr.nr_files; i += 64)
	{
//		read_disk((uint32_t)fcb, );
	}
}
/*
FCB get_fileinfo(char *filename)
{
	int i = 1, j = 0;
	uint32_t inode = 0;
	char name[255], buf[4096];
	FCB fcb;
	while(filename[i])
	{
		if(filename[i] == '/')
		{
			name[j] = 0;
			inode = opendir(inode, dir);
			if(inode == INVALID_NRNODE)
			{
				s
			}
			j = 0;
		}
		name[j ++] = filename[i ++];
	}
	return fcb;
}*/
