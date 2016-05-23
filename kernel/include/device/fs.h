#ifndef __DISK_FORMAT_H__
#define __DISK_FORMAT_H__
/*                                   8KB
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
|   bootmgr  |      bitmap     |    inode    |      file     |
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
     4 KB           512 KB          512 KB       unlimited
*/

#define BLOCKSZ (4 * 1024)

#define L1_ST (12 * BLOCKSZ)
#define L2_ST (L1_ST + 16 * BLOCK_SZ)
#define L3_ST (L2_ST + 16 * 16 * BLOCK_SZ)
#define L4_ST (L3_ST + 16 * 16 * 16 * BLOCK_SZ)

#define L0_PSZ BLOCKSZ
#define L1_PSZ (16 * L0_PSZ)
#define L2_PSZ (16 * L1_PSZ)
#define L3_PSZ (16 * L2_PSZ)

#define L0_SZ L1_ST
#define L1_SZ L2_ST
#define L2_SZ L3_ST
#define L3_SZ L4_ST

#define INVALID_NRNODE 0xFFFFFFFF

#define NR_INODE (1024 * 1024)

#define BITMAP_ST (4096)
#define INODE_ST  (BITMAP_ST + 512 * 1024)
#define FILE_ST   (INODE_ST + 64 * 1024 * 1024)

/* definition of inode */
typedef struct {
	uint32_t nr_block[12];
	uint32_t index_1, index_2, index_3;
	uint32_t pad;
} ROOT_INODE;

typedef struct {
	uint32_t nr_block[16];
} INDEX_L0;

typedef struct {
	uint32_t nr_index_l0[16];
} INDEX_L1;

typedef struct {
	uint32_t nr_index_l1[16];
} INDEX_L2;

typedef struct {
	uint32_t nr_block_l2[16];
} INDEX_L3;

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
	uint32_t pad0, pad1;
} DIR_ATTR;

#endif
