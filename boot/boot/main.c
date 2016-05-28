#include "common.h"

void wait_disk()
{
	//the byte read from 0x1f7 port with bits 0B01xxxxxx indicate the disk is prepared
	while((in_byte(0x1f7) & 0xc0) != 0x40);
}

void read_section(void *dst, int offset) {
	int i;
	/* Issue command */
	wait_disk();
	out_byte(0x1F2, 1);
	out_byte(0x1F3, offset);
	out_byte(0x1F4, offset >> 8);
	out_byte(0x1F5, offset >> 16);
	out_byte(0x1F6, (offset >> 24) | 0xE0);
	out_byte(0x1F7, 0x20);

	/* Fetch data */
	wait_disk();
	for (i = 0; i < 512 / 4; i ++) {
		((int *)dst)[i] = in_long(0x1F0);
	}
}

void readseg(unsigned char *pa, int count, int offset) {
	unsigned char *epa;
	epa = pa + count;
//	pa -= offset % 512;
	offset = (offset / 512) + 8;
	for(; pa < epa; pa += 512, offset ++)
		read_section(pa, offset);
}

void boot_main()
{
	Elf32_Endr *elf;
	Elf32_Phdr *ph, *eph;
	unsigned char* pa, *i;
	void (*entry)(void);

	/* The binary is in ELF format (please search the Internet).
	   0x8000 is just a scratch address. Anywhere would be fine. */
	elf = (Elf32_Endr*)0x8000;

	/* Read the first 4096 bytes into memory.
	   The first several bytes is the ELF header. */
	readseg((unsigned char*)elf, 4096, 0);

	/* Load each program segment */
	ph = (Elf32_Phdr*)((char *)elf + elf->e_phoff);
	eph = ph + elf->e_phnum;
	for(; ph < eph; ph ++) {
		pa = (void *)(ph->p_paddr); /* physical address */
		readseg(pa, ph->p_filesz, ph->p_offset); /* load from disk */
		for (i = pa + ph->p_filesz; i < pa + ph->p_memsz; *i ++ = 0);
	}

	/* Here we go! */
	entry = (void(*)(void))(elf->e_entry);
	entry(); /* never returns */
}
