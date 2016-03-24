#include "common.h"
#include "device/disk.h"
#include "progress/exec.h"

#define DISK_START 102400

int exec(const char *path)
{
	int i, j;
	Elf32_Endr elf;
	Elf32_Phdr ph[10];
	read_disk(((uint32_t)&elf), DISK_START, sizeof(Elf32_Endr));
	assert(elf.e_phnum < 10 && elf.e_phentsize == sizeof(Elf32_Phdr));
	read_disk(((uint32_t)ph), DISK_START + elf.e_phoff, elf.e_phnum * elf.e_phentsize);

	for(i = 0; i < elf.e_phnum; i++)
	{
		read_disk(ph[i].p_paddr, DISK_START + ph[i].p_offset, ph[i].p_filesz);
		for(j = ph[i].p_paddr + ph[i].p_filesz; j < ph[i].p_paddr + ph[i].p_memsz; j++)
		{
			((uint8_t *)j)[0] = 0;
		}
	}

	asm volatile("push %0;ret"::"a"(elf.e_entry));
	return 0;
}
