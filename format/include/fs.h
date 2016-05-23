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
	uint32_t nr_block[12];
	uint32_t index_1, index_2, index_3;
	uint32_t pad;
} ROOT_INODE;

#define ADDRS_PER_BLOCK (BLOCKSZ / sizeof(uint32_t))

#define L1_ST (12 * BLOCKSZ)
#define L2_ST (L1_ST + ADDRS_PER_BLOCK * BLOCKSZ)
#define L3_ST (L2_ST + ADDRS_PER_BLOCK * ADDRS_PER_BLOCK * BLOCKSZ)
#define L4_ST (L3_ST + ADDRS_PER_BLOCK * ADDRS_PER_BLOCK * ADDRS_PER_BLOCK * BLOCKSZ)

#define L0_PSZ BLOCKSZ
#define L1_PSZ (ADDRS_PER_BLOCK * L0_PSZ)
#define L2_PSZ (ADDRS_PER_BLOCK * L1_PSZ)
#define L3_PSZ (ADDRS_PER_BLOCK * L2_PSZ)

#define L0_SZ L1_ST
#define L1_SZ L2_ST
#define L2_SZ L3_ST
#define L3_SZ L4_ST

#define INVALID_NRNODE 0xFFFFFFFF

#define BOOTMGR_SZ (2 * 4096)
#define BITMAP_SZ  (512 * 1024)
#define INODE_SZ   (512 * 1024)

#define NR_INODE (INODE_SZ / 64)

#define BITMAP_ST BOOTMGR_SZ
#define INODE_ST  (BITMAP_ST + BITMAP_SZ)
#define FILE_ST   (INODE_ST + INODE_SZ)


/* definition of file control block in dir file */
typedef struct {
	uint32_t nr_root_inode;
	uint32_t filesz;
	uint32_t filename;
	uint8_t filetype;
	uint8_t fileauthority;
	uint8_t used;
	uint8_t pad;
} FCB;

/* definition for directory file */
typedef struct {
	uint32_t nr_files;
	uint32_t filename_st;
	uint32_t filesz;
	uint32_t pad0;
} DIR_ATTR;

#endif
