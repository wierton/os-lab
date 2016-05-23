#include "common.h"
#include "device/disk.h"
#include "device/fs.h"

/* l0 index: 0 - 6291455
 * l1 index: 6291456 - 14680063
 * l2 index: 14680064 - 148897791
 * l3 index: 148897792 - 229638144
 * l4:unused
 */

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
		moff = FILE_ST + nr_block * BLOCKSZ;
		read_disk((uint32_t)buf, moff, BLOCKSZ);
		buf += BLOCKSZ;
	}

	/* read the last block */
	uint32_t ed_off = FILE_ST + ed_block * BLOCKSZ;
	uint32_t ed_sz = ((offset + size) % BLOCKSZ);
	read_disk((uint32_t)buf, ed_off, ed_sz);

	return size;
}

int fs_read(uint32_t nr_inode, uint32_t offset, uint32_t size, uint8_t *buf)
{
	assert(nr_inode < NR_INODE);
	ROOT_INODE root_inode;
	uint32_t disk_off = INODE_ST + 64 * nr_inode;
	read_disk(((uint32_t)&root_inode), disk_off, sizeof(ROOT_INODE));
	
	if(offset + size < L0_SZ)
	{
		read_l0(&root_inode, offset, size, buf);
	}
	else
	{
		assert(0);
	}
	return size;
}

uint32_t path_translate(char *filename)
{
	return 0;
}
