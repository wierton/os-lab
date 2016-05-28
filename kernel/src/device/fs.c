#include "common.h"
#include "math.h"
#include "string.h"
#include "x86/x86.h"
#include "device/disk.h"
#include "device/fs.h"

uint32_t bitmap[128];
INODE inode[128];

void init_fs()
{
	read_disk(bitmap, BITMAP_ST, sizeof(bitmap));
	read_disk(inode, INODE_ST, sizeof(inode));
}

void clear_block(uint32_t blockno)
{
	char buf[BLOCKSZ];
	memset(buf, 0xFFFFFFFF, sizeof(buf));
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
	uint32_t mask = ~(1 << (31 - (nr_block % 32)));
	bitmap[nr_block / 32] &= mask;
}

INODE *open_inode(uint32_t inodeno)
{
	assert(inodeno < sizeof(inode)/sizeof(inode[0]));;
	return &inode[inodeno];
}

void close_inode(INODE *pinode)
{
	return;
}

uint32_t apply_inode()
{
	int i, j;
	for(i = 1; i < sizeof(inode)/sizeof(inode[0]); i++)
	{
		if(inode[i].used == 0)
		{
			inode[i].filesz = 0;
			inode[i].filetype = '-';
			inode[i].used = 1;
			for(j = 0; j < 13; j++)
				inode[i].nr_block[j] = INVALID_BLOCKNO;
			return i;
		}
	}
	assert(0);
	return INVALID_INODENO;
}

void free_inode(uint32_t nr_inode)
{
	int i;
	assert(nr_inode < sizeof(inode)/sizeof(inode[0]));
	memset(&inode[nr_inode], 0, sizeof(inode[nr_inode]));
	for(i = 0; i < 13; i++)
		inode[nr_inode].nr_block[i] = INVALID_BLOCKNO;
}

int get_disk_blockno(INODE *pinode, uint32_t file_blockst, uint32_t nr_block, uint32_t *buf)
{
	int i, count = 0;
	int level[128] = {0};
	uint32_t addr[128] = {0}, stno[128] = {0};
	uint32_t p = 0, q = 0;
	uint32_t ll[4] = {L1_ST, L2_ST, L3_ST, L4_ST};
	uint32_t lp[4] = {1, L2_ST - L1_ST, L3_ST - L2_ST, L4_ST - L3_ST};
	uint32_t tbuf[BLOCKSZ / sizeof(uint32_t)];

	if(nr_block > 64)
		return 0;

	for(i = 0; i < 10; i++)
		if(i >= file_blockst && i < file_blockst + nr_block)
		{
			level[q] = 0;
			stno[q] = i;
			addr[q++] = pinode->nr_block[i];
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
				stno[q + 1] = stno[p] + (i + 1) * lp[level[q]];
				addr[q] = tbuf[i];
				if(max(stno[q], file_blockst) < min(stno[q + 1], file_blockst + nr_block))
					q++;

			}
			p ++;
			assert(p < 128 && q < 128);
		}
		else
		{
			if(level[p ++] == 0)
				count ++;
			if(count >= nr_block)
			{
				count = 0;
				for(i = 0; i < q; i++)
				{
					if(level[i] == 0)
					{
						buf[count ++] = addr[i];
					}
				}
				return count;
			}
		}
	}
	printk("end of get_disk_fileno\n");
	count = 0;
	for(i = 0; i < q; i++)
	{
		if(level[i] == 0)
		{
			buf[count ++] = addr[i];
		}
	}
	return count;
}

int alloc_disk_blockno(INODE *pinode, uint32_t file_blockst, uint32_t nr_block)
{
	int i, count = 0;
	int level[128] = {0};
	uint32_t addr[128] = {0}, stno[128] = {0};
	uint32_t p = 0, q = 0;
	uint32_t ll[4] = {L1_ST, L2_ST, L3_ST, L4_ST};
	uint32_t lp[4] = {1, L2_ST - L1_ST, L3_ST - L2_ST, L4_ST - L3_ST};
	uint32_t tbuf[BLOCKSZ / sizeof(uint32_t)];

	if(nr_block > 64)
		return 0;

	for(i = 0; i < 10; i++)
		if(i >= file_blockst && i < file_blockst + nr_block)
		{
			level[q] = 0;
			stno[q] = i;
			if(pinode->nr_block[i] == INVALID_BLOCKNO)
				pinode->nr_block[i] = apply_block();
			addr[q++] = pinode->nr_block[i];
		}

	for(i = 0; i < 3; i++)
		if(max(ll[i], file_blockst) < min(ll[i + 1], file_blockst + nr_block))
		{
			level[q] = i + 1;
			stno[q] = ll[i];
			if(pinode->nr_block[10 + i] == INVALID_BLOCKNO)
			{
				pinode->nr_block[10 + i] = apply_block();
				clear_block(pinode->nr_block[10 + i]);
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
				stno[q + 1] = stno[p] + (i + 1) * lp[level[q]];
	
				if(max(stno[q], file_blockst) < min(stno[q + 1], file_blockst + nr_block))
				{
					if(tbuf[i] == INVALID_BLOCKNO)
					{
						tbuf[i] = apply_block();
						if(level[q] > 0)
							clear_block(tbuf[i]);
					}
					addr[q] = tbuf[i];
					q++;
				}
			}
			p ++;
			write_disk(tbuf, disk_off, sizeof(tbuf));
			assert(p < 128 && q < 128);
		}
		else
		{
			assert(p < 128 && q < 128);
			count ++;
			if(count >= nr_block)
			{
				return count;
			}
			p++;
		}
	}
	printk("%d, %d, %d\n", file_blockst, nr_block, count);
	return count;
}

int fs_read(INODE *pinode, uint32_t off, uint32_t size, void *buf)
{
	buf = (uint8_t *)buf;
	int i, j, tsize, ed;
	uint32_t stblockno, edblockno;
	uint32_t block_addr[64];
	uint32_t stno = off / BLOCKSZ, edno = (off + size - 1) / BLOCKSZ;
	if(off >= pinode->filesz || size == 0)
		return 0;
	size = min(pinode->filesz, off + size) - off;
	ed = off + size;

	/* read the first block(given that some read operation on bytes less than 4096) */
	tsize = min(BLOCKSZ - off % BLOCKSZ, size);
	get_disk_blockno(pinode, stno, 1, &stblockno);
	read_disk(buf, FILE_ST + stblockno * BLOCKSZ + off % BLOCKSZ, tsize);
	buf += tsize;
	off += tsize;
	size -= tsize;
	for(i = stno + 1; i < edno; i += 64)
	{
		int t = get_disk_blockno(pinode, i, min(64, edno - i), block_addr);
		for(j = 0; j < t; j++)
		{
			read_disk(buf, FILE_ST + block_addr[j] * BLOCKSZ, BLOCKSZ);
			buf += BLOCKSZ;
			off += BLOCKSZ;
			size -= BLOCKSZ;
		}
	}
	/* read the last block */
	tsize = ed - off;
	get_disk_blockno(pinode, edno, 1, &edblockno);
	read_disk(buf, FILE_ST + edblockno * BLOCKSZ + off % BLOCKSZ, tsize);
	buf += tsize;
	off += tsize;
	size -= tsize;

	return size;
}

int fs_write(INODE *pinode, uint32_t off, uint32_t size, void *buf)
{
	buf = (uint8_t *)buf;
	int i, j, tsize, ed = off + size;
	uint32_t block_addr[64];
	if(off > pinode->filesz || size == 0)
		return 0;

	if(off < pinode->filesz)
	{
		/* write the part haven't exceed the file end
		 * write scale [off, min(filesz, off + size)
		 */
		uint32_t stblockno, edblockno;
		uint32_t preved = min(off + size, pinode->filesz);
		uint32_t stno = off / BLOCKSZ, edno = (preved - 1) / BLOCKSZ;
		/* write the first block */
		tsize = min(BLOCKSZ - off % BLOCKSZ, size);
		get_disk_blockno(pinode, stno, 1, &stblockno);
		write_disk(buf, FILE_ST + stblockno * BLOCKSZ + off % BLOCKSZ, tsize);
		buf += tsize;
		off += tsize;
		size -= tsize;
		for(i = stno + 1; i < edno; i += 64)
		{
			int t = get_disk_blockno(pinode, i, min(i + 64, edno - i), block_addr);
			for(j = 0; j < t; j++)
			{
				write_disk(buf, FILE_ST + block_addr[j] * BLOCKSZ, BLOCKSZ);
				buf += BLOCKSZ;
				off += BLOCKSZ;
				size -= BLOCKSZ;
			}
		}
		
		/* write the last block */
		tsize = preved - off;
		get_disk_blockno(pinode, edno, 1, &edblockno);
		write_disk(buf, FILE_ST + edblockno * BLOCKSZ + off % BLOCKSZ, tsize);
		buf += tsize;
		off += tsize;
		size -= tsize;
	}
	if(ed > pinode->filesz)
	{
		/* write the part which need to apply new block
		 * write scale [off, ed)
		 */

		off = pinode->filesz;
		uint32_t stblockno, edblockno;
		uint32_t stno = off / BLOCKSZ, edno = (ed - 1) / BLOCKSZ;
		/* alloc block for file */
		for(i = stno; i <= edno; i += 64)
			alloc_disk_blockno(pinode, i, min(64, edno - i + 1));
		/* write the first block */
		tsize = min(BLOCKSZ - off % BLOCKSZ, size);

		get_disk_blockno(pinode, stno, 1, &stblockno);
		write_disk(buf, FILE_ST + stblockno * BLOCKSZ + off % BLOCKSZ, tsize);
		buf += tsize;
		off += tsize;
		size -= tsize;
		for(i = stno + 1; i < edno; i += 64)
		{
			int t = get_disk_blockno(pinode, i, min(64, edno - i), block_addr);
			for(j = 0; j < t; j++)
			{
				write_disk(buf, FILE_ST + block_addr[j] * BLOCKSZ, BLOCKSZ);
				buf += BLOCKSZ;
				off += BLOCKSZ;
				size -= BLOCKSZ;
			}
		}
		
		/* write the last block */
		tsize = ed - off;
		get_disk_blockno(pinode, edno, 1, &edblockno);
		write_disk(buf, FILE_ST + edblockno * BLOCKSZ + off % BLOCKSZ, tsize);
		buf += tsize;
		off += tsize;
		size -= tsize;
	}

	pinode->filesz = max(pinode->filesz, ed);
	
	return size;
}


/* function for directory : 4 * 32B = 64B (variable-length filename)
 * +---------------------------------------------------------+
 * |                     dir attr                            |
 * +---------------------------------------------------------+
 * |  inode_1   | filename_st |  len_of_name  |    dirty     |
 * +---------------------------------------------------------+
 * |  inode_2   | filename_st |  len_of_name  |    dirty     |
 * +---------------------------------------------------------+
 * |  inode_3   | filename_st |  len_of_name  |    dirty     |
 * +---------------------------------------------------------+
 * |                         ...                             |
 * +---------------------------------------------------------+
 * |                collection of filename                   |
 * +---------------------------------------------------------+
 * |                         ...                             |
 * +---------------------------------------------------------+
 * */

void addto_dir(INODE *pdirinode, INODE *pfinode, char *filename)
{
	int i, j, index = -1;
	DIR_ATTR da = {0};
	FILE_ATTR dfa = {0}, fa[32] = {{0}};
	int len = strlen(filename);
	char name[255];

	/* read directory attribute */
	if(pdirinode->filesz == 0)
	{
		da.nr_files = 1;
		da.nr_index = 1;
		da.filesz = sizeof(DIR_ATTR);
		fs_write(pdirinode, 0, sizeof(DIR_ATTR), &da);
	}
	else
	{
		fs_read(pdirinode, 0, sizeof(DIR_ATTR), &da);
		da.nr_files ++;
	}

	/* init destination fileattr */
	dfa.inode = pfinode->inodeno;
	dfa.len = len;

	for(i = 0; i < da.nr_index; i += 32)
	{
		int left = min(sizeof(fa), (da.nr_index - i) * sizeof(FILE_ATTR));
		fs_read(pdirinode, sizeof(DIR_ATTR) + i * sizeof(FILE_ATTR), left, (void*)fa);
		for(j = 0; j < left / sizeof(FILE_ATTR); j++)
		{
			if(fa[j].dirty == 0)
			{
				index = i + j;
				fa[j].dirty = 1;
				goto exitloop_addto_dir;
			}
		}
	}
exitloop_addto_dir:

	if(index == -1)
	{
		index = da.nr_index;
		for(i = 0; i < da.nr_index; i += 32)
		{
			bool dirty = false;
			int left = min(sizeof(fa), (da.nr_index - i) * sizeof(FILE_ATTR));
			int fop = sizeof(DIR_ATTR) + da.nr_index * sizeof(FILE_ATTR);
			fs_read(pdirinode, sizeof(DIR_ATTR) + i * sizeof(FILE_ATTR), left, fa);
			for(j = 0; j < left / sizeof(FILE_ATTR); j++)
			{
				/* move filename in nr_index's index */
				if(fa[j].filename_st >= fop && fa[j].filename_st < fop + sizeof(FILE_ATTR))
				{
					int t = max(pdirinode->filesz, fop + sizeof(FILE_ATTR));
					if(t > pdirinode->filesz)
					{
						memset(name, 0, t - pdirinode->filesz);
						fs_write(pdirinode, pdirinode->filesz, t - pdirinode->filesz, name);
					}
					fs_read(pdirinode, fa[j].filename_st, fa[j].len + 1, name);
					fa[j].filename_st = t;
					fs_write(pdirinode, t, fa[j].len + 1, name);
					dirty = true;
	
				}
			}
			
			/* write back data if dirty */
			if(dirty)
				fs_write(pdirinode, sizeof(DIR_ATTR) + i * sizeof(FILE_ATTR), left, fa);
		}

		da.nr_index ++;
	}

	/* update dirattr */
	fs_write(pdirinode, 0, sizeof(DIR_ATTR), (void *)&da);
	/* write dest fileattr */
	dfa.dirty = 1;
	fs_write(pdirinode, sizeof(DIR_ATTR) + index * sizeof(FILE_ATTR), sizeof(FILE_ATTR), &dfa);

	/* write filename */
	fs_write(pdirinode, pdirinode->filesz, len + 1, filename);
	fs_read(pdirinode, pdirinode->filesz - len - 1, len + 1, name);

	/* write again to update filename_st */
	dfa.filename_st = pdirinode->filesz - len - 1;
	fs_write(pdirinode, sizeof(DIR_ATTR) + index * sizeof(FILE_ATTR), sizeof(FILE_ATTR), &dfa);
}

void delfrom_dir(INODE *pdirinode, char *filename)
{
	int i, j;
	DIR_ATTR da = {0};
	FILE_ATTR fa[32] = {{0}};
	char name[255];

	/* read directory attribute */
	fs_read(pdirinode, 0, sizeof(DIR_ATTR), &da);
	da.nr_files --;

	for(i = 0; i < da.nr_index; i += 32)
	{
		int left = min(sizeof(fa), (da.nr_index - i) * sizeof(FILE_ATTR));
		fs_read(pdirinode, sizeof(DIR_ATTR) + i * sizeof(FILE_ATTR), left, fa);
		for(j = 0; j < left / sizeof(FILE_ATTR); j++)
		{
			if(fa[j].dirty == 1)
			{
				fs_read(pdirinode, fa[j].filename_st, fa[j].len + 1, name);
				if(strcmp(name, filename) == 0)
				{
					fa[j].dirty = 0;
					fs_write(pdirinode, sizeof(DIR_ATTR) + i * sizeof(FILE_ATTR), left, fa);
					return;
				}
			}
		}
	}
}

uint32_t opendir(char *filename)
{
	int i, j;
	DIR_ATTR da = {0};
	FILE_ATTR fa[32] = {{0}};
	char name[255], path[255];
	int is_dir = 0, p = 1, len = strlen(filename);
	INODE *pinode = open_inode(0);

	if(filename[0] != '/')
	{
		return INVALID_INODENO;
	}

	if(len == 1)
	{
		int t = pinode->inodeno;
		close_inode(pinode);
		return t;
	}

	while(p < len)
	{
		int find_sub = 0;
		is_dir = 0;
		for(i = p; i <= len; i++)
		{
			if(filename[i] == '/')
			{
				is_dir = 1;
				path[i - p] = '\0';
				break;
			}
			path[i - p] = filename[i];
		}
		p = i + 1;

		/* read directory attribute */
		fs_read(pinode, 0, sizeof(DIR_ATTR), &da);
	
		for(i = 0; i < da.nr_index; i += 32)
		{
			int left = min(sizeof(fa), (da.nr_index - i) * sizeof(FILE_ATTR));
			fs_read(pinode, sizeof(DIR_ATTR) + i * sizeof(FILE_ATTR), left, fa);
			for(j = 0; j < left / sizeof(FILE_ATTR); j++)
			{
				if(fa[j].dirty == 1)
				{
					fs_read(pinode, fa[j].filename_st, fa[j].len + 1, name);
					/* find file with the same name of path */
					if(strcmp(name, path) == 0)
					{
						close_inode(pinode);
						pinode = open_inode(fa[j].inode);
						if(is_dir && pinode->filetype == 'd')
						{
							find_sub = 1;
							if(p >= len)
								return pinode->inodeno;
							break;
						}
						if(!is_dir && pinode->filetype == '-')
							return fa[j].inode;
					}
				}
			}
			if(find_sub)
				break;
		}

		if(!find_sub)
		{
			close_inode(pinode);
			break;
		}
	}

	return INVALID_INODENO;
}

int creat(char *pathname)
{
	char path[255];
	int i = 0, p = 0;
	int ret = opendir(pathname);
	if(ret != INVALID_INODENO || pathname[0] != '/')
		return -1;

	while(pathname[i])
	{
		if(pathname[i ++] == '/')
			p = i;
	}

	for(i = 0; i < p; i++)
		path[i] = pathname[i];
	path[i] = '\0';

	int inodeno = opendir(path);
	if(inodeno != INVALID_INODENO)
	{
		INODE *pdirinode = open_inode(inodeno);
		uint32_t finode = apply_inode();
		INODE *pfinode = open_inode(finode);
		pfinode->used = 1;
		pfinode->filesz = 0;
		pfinode->filetype = '-';
		addto_dir(pdirinode, pfinode, &pathname[p]);
		close_inode(pfinode);
		close_inode(pdirinode);
		return 0;
	}

	return -1;
}

int makedir(char *pathname)
{
	char path[255];
	int i = 0, p;
	int ret = opendir(pathname);
	if(ret != INVALID_INODENO || pathname[0] != '/')
		return -1;

	while(pathname[i])
	{
		if(pathname[i ++] == '/')
			p = i;
	}

	for(i = 0; i < p; i++)
		path[i] = pathname[i];
	path[i] = '\0';

	int inodeno = opendir(path);
	if(inodeno != INVALID_INODENO)
	{
		INODE *pdirinode = open_inode(inodeno);
		uint32_t finode = apply_inode();
		INODE *pfinode = open_inode(finode);
		pfinode->filesz = 0;
		pfinode->filetype = 'd';
		addto_dir(pdirinode, pfinode, &pathname[p]);
		close_inode(pfinode);
		close_inode(pdirinode);
		return 0;
	}

	return -1;
}

int list(TrapFrame *tf)
{
	int i, j, total_size = 0, count = 0;
	char *name[255];
	DIR_ATTR da = {0};
	FILE_ATTR fa[32] = {{0}};
	char *path = (char *)tf->ebx;
	uint32_t inodeno = opendir(path);
	if(inodeno == INVALID_INODENO)
		return -1;

	INODE *pinode = open_inode(inodeno);

	fs_read(pinode, 0, sizeof(DIR_ATTR), &da);

	for(i = 0; i < da.nr_index; i += 32)
	{
		int left = min(sizeof(fa), (da.nr_index - i) * sizeof(FILE_ATTR));
		fs_read(pinode, sizeof(DIR_ATTR) + i * sizeof(FILE_ATTR), left, fa);
		for(j = 0; j < left / sizeof(FILE_ATTR); j++)
		{
			if(fa[j].dirty == 1)
			{
				fs_read(pinode, fa[j].filename_st, fa[j].len + 1, name);
				INODE *pfinode = open_inode(fa[j].inode);
				printk("%c %d\t%s\n", pfinode->filetype, pfinode->filesz, name);
				total_size += pfinode->filesz;
				count ++;
				close_inode(pfinode);
			}
		}
	}

	printk("total:%d, size:%d\n\n", count, total_size);
	close_inode(pinode);
	return 0;
}
