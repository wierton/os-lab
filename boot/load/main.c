#include "common.h"
#include "string.h"
#include "fs.h"

#define va_to_pa(addr) ((addr) - 0xc0000000)

static char *dststr = NULL;
typedef void (*PRINTER)(char);

#define assert(cond) \
	do { \
		if(!(cond)) \
		{ \
			printk("assertion fail: %d!", __LINE__); \
			while(1); \
		} \
	} while(0)

void init_serial()
{
	out_byte(SERIAL_PORT + 1, 0x00);
	out_byte(SERIAL_PORT + 3, 0x80);
	out_byte(SERIAL_PORT + 0, 0x01);
	out_byte(SERIAL_PORT + 1, 0x00);
	out_byte(SERIAL_PORT + 3, 0x03);
	out_byte(SERIAL_PORT + 2, 0xC7);
	out_byte(SERIAL_PORT + 4, 0x0B);
}

static inline int serial_idle()
{
	return (in_byte(SERIAL_PORT + 5) & 0x20) != 0;
}

void serial_printc(char ch)
{
	while (serial_idle() != 1);
	out_byte(SERIAL_PORT, ch);
}

void sprintc(char ch)
{
	if(dststr != NULL)
		*dststr++ = ch;
}

inline void prints(char *str, PRINTER printer)
{
	if(str != NULL)
		for(;*str != '\0'; str ++) printer(*str);
}

void printd(int val, PRINTER printer)
{
	int i = 15;
	bool IsNeg = false;
	char buf[20] = {0};
	if(val < 0)
	{
		IsNeg = true;
	}
	do
	{
		int temp = val % 10;
		temp = temp < 0 ? (-temp) : temp;
		buf[i--] = '0' + temp;
		val /= 10;
	} while(val);
	
	if(IsNeg)
		buf[i--] = '-';

	prints(&buf[i + 1], printer);
}

void printx(uint32_t val, PRINTER printer)
{
	int i, pos = 0;
	char buf[20];
	bool IsPrefix = true;
	for(i = 7; i >= 0; i --)
	{
		uint8_t temp = (val >> (4 * i)) & 0xf;
		if(temp == 0 && IsPrefix)
			continue;
		IsPrefix = false;
		if(temp < 0xa)
			buf[pos ++] = '0' + temp;
		else
			buf[pos ++] = 'a' + temp - 0xa;
	}
	if(pos == 0)
		buf[pos ++] = '0';
	buf[pos] = 0;
	prints((void *)buf, printer);
}

int vfprintf(const char *ctl, void **args, PRINTER printer) {
	int i = 0, pargs = 0;
	for(;ctl[i] != '\0'; i ++)
	{
		if(ctl[i] != '%')
		{
			printer(ctl[i]);
			continue;
		}
		switch(ctl[++ i])
		{
			case 'c':
			case 'C':
				printer((char)(((int *)args)[pargs ++]));
				break;
			case 'd':
			case 'D':
				printd((((int *)args)[pargs ++]), printer);
				break;
			case 's':
			case 'S':
				prints((void *)(((int *)args)[pargs ++]), printer);
				break;
			case 'p':
			case 'P':
			case 'x':
			case 'X':
				printx((((uint32_t *)args)[pargs ++]), printer);
				break;
			default:
				break;
		}
	}
	return 0;
}

void __attribute__((noinline)) printk(const char *ctl, ...)
{
	void **args = ((void **)(&ctl)) + 1;
	vfprintf(ctl, args, serial_printc);
}

void __attribute__((noinline)) sprintk(char *dst, const char *ctl, ...)
{
	void **args = ((void **)(&ctl)) + 1;
	dststr = dst;
	vfprintf(ctl, args, sprintc);
	sprintc('\0');
	dststr = NULL;
}

void wait_disk()
{
	while((in_byte(0x1f7) & 0xc0) != 0x40);
}

void read_section(uint32_t dst, int sectnum)
{
	wait_disk();
	out_byte(0x1f2, 1);
	out_byte(0x1f3, sectnum);
	out_byte(0x1f4, sectnum >> 8);
	out_byte(0x1f5, sectnum >> 16);
	out_byte(0x1f6, (sectnum >> 24) | 0xE0);
	out_byte(0x1f7, 0x20);
	wait_disk();
	uint32_t tar = dst + 512;
	for(;dst < tar; dst += 4)
	{
		((uint32_t *)dst)[0] = in_long(0x1f0);
	}
}

int read_disk(void *dst, uint32_t offset, uint32_t size)
{
	int i, j, op = 0, ed = 0;
	uint32_t off = offset / 512;
	uint32_t final = (uint32_t)dst + size;
	i = (uint32_t)dst - offset % 512;

	uint8_t sect[512];
	for(i = (uint32_t)dst - offset % 512; i < final; i += 512)
	{
		read_section((uint32_t)sect, off++);
		op = max(i, (uint32_t)dst);
		ed = min(i + 512, final);
		for(j = op; j < ed; j++)
		{
			((uint8_t *)j)[0] = sect[j - i];
		}
	}
	return size;
}

INODE __attribute__((noinline)) *open_inode(uint32_t inodeno)
{
	static INODE inode;
	read_disk(&inode, INODE_ST + inodeno * sizeof(INODE), sizeof(INODE));
	return &inode;
}

void close_inode(INODE *pinode)
{
	return;
}

int get_disk_blockno(INODE *pinode, uint32_t file_blockst, uint32_t nr_block, uint32_t *buf)
{
	int i, count = 0;
	int level[1024] = {0};
	uint32_t addr[1024] = {0}, stno[1024] = {0};
	uint32_t p = 0, q = 0;
	uint32_t ll[4] = {L1_ST, L2_ST, L3_ST, L4_ST};
	uint32_t lp[4] = {1, L2_ST - L1_ST, L3_ST - L2_ST, L4_ST - L3_ST};
	uint32_t tbuf[BLOCKSZ / sizeof(uint32_t)];

	if(nr_block > 512)
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
			assert(p < 1024 && q < 1024);
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

int fs_read(INODE *pinode, uint32_t off, uint32_t size, void *buf)
{
	buf = (uint8_t *)buf;
	int i, j, tsize, ed;
	uint32_t stblockno, edblockno;
	uint32_t block_addr[256];
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
	for(i = stno + 1; i < edno; i += 256)
	{
		int t = get_disk_blockno(pinode, i, min(256, edno - i), block_addr);
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

void loader()
{
	int i, j;
	Elf32_Endr *elf = (Elf32_Endr *)0x3000;
	Elf32_Phdr *ph;

	printk("Load Kernel!\n");
	uint32_t inodeno = opendir("/kernel");
	INODE *pinode = open_inode(inodeno);
	
	fs_read(pinode, 0, 0x1000, elf);
	ph = (void *)elf + elf->e_phoff;
	printk("%x %d\n", elf->e_entry, pinode->inodeno);

	for(i = 0; i < elf->e_phnum; i++)
	{
		fs_read(pinode, ph->p_offset, ph->p_filesz, (void *)va_to_pa(ph->p_vaddr));
		/*zero the memory [paddr + filesz, paddr + memsz)*/
		for(j = va_to_pa(ph->p_vaddr) + ph->p_filesz; j < va_to_pa(ph->p_vaddr) + ph->p_memsz; j++)
		{
			((uint8_t *)j)[0] = 0;
		}
		ph++;
	}

	asm volatile("push %0;ret"::"a"(va_to_pa(elf->e_entry)));
}
