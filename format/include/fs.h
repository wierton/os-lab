#ifndef __DISK_FORMAT_H__
#define __DISK_FORMAT_H__
/*                                   8KB
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
|  mbr  |   bootmgr  |      bitmap     |    inode    |      file     |
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   4KB       4 KB           512 KB          512 KB       unlimited
*/

#define BLOCKSZ (4 * 1024)

/* l0   0 - 49511
 * l1   49512 - 114687
 * l2   114688 - 
 */

/* definition of inode */
typedef struct {
	uint32_t filesz;
	uint8_t filetype;
	uint8_t used;
	uint32_t nr_block[10 + 3];
	uint32_t pad;
} INODE;

#define ADDRS_PER_BLOCK (BLOCKSZ / sizeof(uint32_t))

#define L1_ST 10
#define L2_ST (L1_ST + ADDRS_PER_BLOCK)
#define L3_ST (L2_ST + ADDRS_PER_BLOCK * ADDRS_PER_BLOCK)
#define L4_ST (L3_ST + ADDRS_PER_BLOCK * ADDRS_PER_BLOCK * ADDRS_PER_BLOCK)

#define L0_PSZ BLOCKSZ
#define L1_PSZ (ADDRS_PER_BLOCK * L0_PSZ)
#define L2_PSZ (ADDRS_PER_BLOCK * L1_PSZ)
#define L3_PSZ (ADDRS_PER_BLOCK * L2_PSZ)

#define L0_SZ (L1_ST * BLOCKSZ)
#define L1_SZ (L2_ST * BLOCKSZ)
#define L2_SZ (L3_ST * BLOCKSZ)
#define L3_SZ (L4_ST * BLOCKSZ)

#define INVALID_NRINODE 0xFFFFFFFF
#define INVALID_NRBLOCK 0

#define BOOTMGR_SZ (2 * 4096)
#define BITMAP_SZ  (512 * 1024)
#define INODE_SZ   (512 * 1024)

#define NR_INODE (INODE_SZ / 64)

#define BITMAP_ST BOOTMGR_SZ
#define INODE_ST  (BITMAP_ST + BITMAP_SZ)
#define FILE_ST   (INODE_ST + INODE_SZ)

/* definition for directory file */
typedef struct {
	uint32_t nr_files;
	uint32_t filename_st;
	uint32_t filesz;
	uint32_t pad0;
} DIR_ATTR;

int read_disk(void *buf, uint32_t off, uint32_t size);
int write_disk(void *buf, uint32_t off, uint32_t size);
void init_disk(int argv, char *args[]);
void zeros_block(uint32_t blockno);
uint32_t apply_block();
void free_block(uint32_t nr_block);
uint32_t apply_inode();
void free_inode(uint32_t nr_inode);
int get_disk_blockno(INODE *pinode, uint32_t file_blockst, uint32_t nr_block, uint32_t *buf);
int alloc_disk_blockno(INODE *pinode, uint32_t file_blockst, uint32_t nr_block);
int fs_read(INODE *pinode, uint32_t off, uint32_t size, uint8_t *buf);
int fs_write(INODE *pinode, uint32_t off, uint32_t size, uint8_t *buf);

#endif
