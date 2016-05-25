#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "common.h"
#include "fs.h"

#define log(format, ...) printf("%s %d: "format, __func__, __LINE__, __VA_ARGS__)

uint32_t bitmap[512 * 1024 / 32];
INODE inode[512 * 1024 / sizeof(INODE)];

int read_disk(void *buf, uint32_t off, uint32_t size)
{
	FILE *fp = fopen("disk.img", "r");
	fseek(fp, off, SEEK_SET);
	uint32_t ret = fread(buf, size, 1, fp);
	fclose(fp);
	return ret;
}

int write_disk(void *buf, uint32_t off, uint32_t size)
{
	FILE *fp = fopen("disk.img", "w");
	if(fp == NULL)
		printf("\033[1;31mFail to open disk.img\033[0m\n");
	fseek(fp, off, SEEK_SET);
	printf("%d, %x ", size, off);
	uint32_t ret = fwrite(buf, size, 1, fp);
	printf("%d\n", ret);
	fclose(fp);
	return ret;
}

void init_disk(int argv, char *args[])
{
	memset(bitmap, 0, sizeof(bitmap));
	memset(inode, 0, sizeof(inode));
	bitmap[0] = 0x80000000;
}

void zeros_block(uint32_t blockno)
{
	char buf[BLOCKSZ];
	memset(buf, 0, sizeof(buf));
	write_disk(buf, FILE_ST + blockno * BLOCKSZ, BLOCKSZ);
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
					uint32_t mask = 1 << j;
					bitmap[i] |= mask;
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

uint32_t apply_inode()
{
	int i;
	for(i = 0; i < sizeof(inode)/sizeof(inode[0]); i++)
	{
		if(inode[i].used == 0)
		{
			inode[i].used = 1;
			return i;
		}
	}
	assert(0);
	return INVALID_NRINODE;
}

void free_inode(uint32_t nr_inode)
{
	assert(nr_inode < sizeof(inode)/sizeof(inode[0]));
	memset(&inode[nr_inode], 0, sizeof(inode[nr_inode]));
}

int get_disk_blockno(INODE *pinode, uint32_t file_blockst, uint32_t nr_block, uint32_t *buf)
{
	int i, j, count = 0;
	char level[1024];
	uint32_t addr[1024], stno[1024];
	uint32_t p = 0, q = 0;
	uint32_t ll[4] = {L1_ST, L2_ST, L3_ST, L4_ST};
	uint32_t lp[4] = {1, L2_ST - L1_ST, L3_ST - L2_ST, L4_ST - L3_ST};
	uint32_t tbuf[BLOCKSZ / sizeof(uint32_t)];

	for(i = 0; i < 10; i++)
		if(i >= file_blockst && i < file_blockst + nr_block)
		{
			level[q] = 0;
			stno[q] = i;
			addr[q++] = pinode->nr_block[i];
			count ++;
		}

	for(i = 0; i < 3; i++)
		if(max(ll[i], file_blockst) < min(ll[i + 1], file_blockst + nr_block))
		{
			level[q] = i + 1;
			stno[q] = ll[i];
			addr[q++] = pinode->nr_block[10 + i];
		}

	if(q == 0)
		return 0;

	while(p != q)
	{
		if(level[p] > 0)
		{
			uint32_t disk_off = FILE_ST + addr[p] * 4 * 1024;
			read_disk(tbuf, disk_off, sizeof(tbuf));
			for(i = 0; i < sizeof(tbuf)/sizeof(uint32_t); i++)
			{
				level[q] = level[p] - 1;
				stno[q] = stno[p] + i * lp[level[q]];
				addr[q] = tbuf[i];
				if(stno[q] >= file_blockst && stno[q] < file_blockst + nr_block)
					q++;
				if(level[q] == 0)
					count ++;
				if(count > nr_block)
				{
					log("%d,%d\n", stno[q], file_blockst + nr_block);
					count = 0;
					for(j = 0; j < q; j++)
					{
						if(level[j] == 0)
						{
							buf[count ++] = addr[j];
						}
					}
					return count;
				}
			}
			assert(p < 1024 && q < 1024);
			level[p ++] = -1;
		}
	}
	printf("end of get_disk_fileno\n");
	count = 0;
	for(j = 0; j < q; j++)
	{
		if(level[j] == 0)
		{
			buf[count ++] = addr[j];
		}
	}
	return count;
}

int alloc_disk_blockno(INODE *pinode, uint32_t file_blockst, uint32_t nr_block)
{
	int i, j, count = 0;
	char level[1024];
	uint32_t addr[1024], stno[1024];
	uint32_t p = 0, q = 0;
	uint32_t ll[4] = {L1_ST, L2_ST, L3_ST, L4_ST};
	uint32_t lp[4] = {1, L2_ST - L1_ST, L3_ST - L2_ST, L4_ST - L3_ST};
	uint32_t tbuf[BLOCKSZ / sizeof(uint32_t)];

	for(i = 0; i < 10; i++)
		if(i >= file_blockst && i < file_blockst + nr_block)
		{
			level[q] = 0;
			stno[q] = i;
			if(pinode->nr_block[i] == INVALID_NRBLOCK)
				pinode->nr_block[i] = apply_block();
			addr[q++] = pinode->nr_block[i];
			count ++;
		}

	for(i = 0; i < 3; i++)
		if(max(ll[i], file_blockst) < min(ll[i + 1], file_blockst + nr_block))
		{
			level[q] = i + 1;
			stno[q] = ll[i];
			if(pinode->nr_block[10 + i] == INVALID_NRBLOCK)
			{
				pinode->nr_block[10 + i] = apply_block();
				zeros_block(pinode->nr_block[10 + i]);
			}
			addr[q++] = pinode->nr_block[10 + i];
		}

	if(q == 0)
		return 0;

	while(p < q)
	{
		if(level[p] > 0)
		{
			uint32_t disk_off = FILE_ST + addr[p] * BLOCKSZ;
			read_disk(tbuf, disk_off, sizeof(tbuf));
			for(i = 0; i < sizeof(tbuf)/sizeof(uint32_t); i++)
			{
				level[q] = level[p] - 1;
				stno[q] = stno[p] + i * lp[level[q]];

				if(stno[q] >= file_blockst && stno[q] < file_blockst + nr_block)
				{
					if(tbuf[i] == INVALID_NRBLOCK)
					{
						tbuf[i] = apply_block();
						if(level[q] > 0)
							zeros_block(tbuf[i]);
					}
					addr[q] = tbuf[i];
					q++;
				}
			}
			p ++;
			write_disk(tbuf, disk_off, sizeof(tbuf));
			assert(p < 1024 && q < 1024);
		}
		else
		{
			if(level[p] == 0)
				count ++;
			if(count >= nr_block)
			{
				return count;
			}
			p++;
		}
	}
	return count;
}

int fs_read(INODE *pinode, uint32_t off, uint32_t size, uint8_t *buf)
{
	int i, j, tsize, ed = off + size;
	uint32_t block_addr[256];
	uint32_t stno = off / BLOCKSZ, edno = (off + size - 1) / BLOCKSZ;
	if(off > pinode->filesz || size == 0)
		return 0;
	size = min(pinode->filesz, off + size);

	/* read the first block(given that some read operation on bytes less than 4096) */
	tsize = min(BLOCKSZ - off % BLOCKSZ, size);
	read_disk(buf, off, tsize);
	buf += tsize;
	off += tsize;
	for(i = stno + 1; i < edno; i += 256)
	{
		int t = get_disk_blockno(pinode, i, min(i + 256, edno - 1), block_addr);
		log("%d, %d, %d, %d\n", t, i, i + 256, edno);
		for(j = 0; j < t; j++)
		{
			read_disk(buf, FILE_ST + block_addr[j] * BLOCKSZ, BLOCKSZ);
			buf += BLOCKSZ;
			off += BLOCKSZ;
		}
	}
	/* read the last block */
	int toff = max(off, ed - tsize);
	read_disk(buf, toff, ed - toff);
	buf += tsize;
	off += tsize;

	return size;
}

int fs_write(INODE *pinode, uint32_t off, uint32_t size, uint8_t *buf)
{
	int i, j, tsize, ed = off + size;
	uint32_t block_addr[256];
	if(off > pinode->filesz || size == 0)
		return 0;

	log("%d, %d\n", off, pinode->filesz);
	if(off < pinode->filesz)
	{
		/* write the part haven't exceed the file end
		 * write scale [off, min(filesz, off + size)
		 */
		uint32_t preved = min(off + size, pinode->filesz);
		uint32_t stno = off / BLOCKSZ, edno = (preved - 1) / BLOCKSZ;
		/* write the first block */
		tsize = min(BLOCKSZ - off % BLOCKSZ, size);
		write_disk(buf, off, tsize);
		buf += tsize;
		off += tsize;
		for(i = stno + 1; i < edno; i += 256)
		{
			int t = get_disk_blockno(pinode, i, min(i + 256, edno - 1), block_addr);
			log("%d, %d, %d, %d\n", t, i, i + 256, edno);
			for(j = 0; j < t; j++)
			{
				write_disk(buf, FILE_ST + block_addr[j] * BLOCKSZ, BLOCKSZ);
				buf += BLOCKSZ;
				off += BLOCKSZ;
			}
		}
		
		/* write the last block */
		int toff = max(off, ed - tsize);
		read_disk(buf, toff, ed - toff);
		buf += tsize;
		off += tsize;
	}
	if(ed > pinode->filesz)
	{
		/* write the part which need to apply new block
		 * write scale [off, ed)
		 */

		log("%d, %d, %d\n", off, size, pinode->filesz);

		off = pinode->filesz;
		uint32_t stno = off / BLOCKSZ, edno = (ed - 1) / BLOCKSZ;
		log("%d, %d\n", stno, edno);
		alloc_disk_blockno(pinode, stno, edno - stno + 1);
		/* write the first block */
		tsize = min(BLOCKSZ - off % BLOCKSZ, size);
		write_disk(buf, off, tsize);
		buf += tsize;
		off += tsize;
		for(i = stno + 1; i < edno; i += 256)
		{
			int t = get_disk_blockno(pinode, i, min(i + 256, edno - 1), block_addr);
			log("%d, %d, %d, %d\n", t, i, i + 256, edno);
			for(j = 0; j < t; j++)
			{
				write_disk(buf, FILE_ST + block_addr[j] * BLOCKSZ, BLOCKSZ);
				buf += BLOCKSZ;
				off += BLOCKSZ;
			}
		}
		
		/* write the last block */
		int toff = max(off, ed - tsize);
		read_disk(buf, toff, ed - toff);
		buf += tsize;
		off += tsize;
	}

	pinode->filesz = max(pinode->filesz, off + size);
	
	return size;
}
