#include "common.h"
#include "x86/x86.h"
#include "x86/memory.h"
#include "device/disk.h"
#include "process/exec.h"

#define DISK_START 102400
#define GAME_STACK_ADDR 0x50000000
#define GAME_STACK_SIZE (1 << 22)

uint32_t load_elf(uint32_t disk_start)
{
	int i, j;
	Elf32_Endr elf;
	Elf32_Phdr ph[10];
	read_disk(((uint32_t)&elf), disk_start, sizeof(Elf32_Endr));
	assert(elf.e_phnum < 10 && elf.e_phentsize == sizeof(Elf32_Phdr));
	read_disk(((uint32_t)ph), disk_start + elf.e_phoff, elf.e_phnum * elf.e_phentsize);

	HANDLE hGame = apply_udir();
	mm_alloc(hGame, GAME_STACK_ADDR - GAME_STACK_SIZE, GAME_STACK_SIZE);
	load_udir(hGame);

	/* load into memory */
	for(i = 0; i < elf.e_phnum; i++)
	{
		mm_alloc(hGame, ph[i].p_vaddr, ph[i].p_memsz);
		for(j = ph[i].p_vaddr; j < ph[i].p_vaddr + ph[i].p_memsz; j += 4)
		{
			/* make sure it can be read and write */
			((uint32_t *)j)[0] = j;
		}
	}

	for(i = 0; i < elf.e_phnum; i++)
	{
		read_disk(ph[i].p_vaddr, disk_start + ph[i].p_offset, ph[i].p_filesz);
		for(j = ph[i].p_vaddr + ph[i].p_filesz; j < ph[i].p_vaddr + ph[i].p_memsz; j++)
		{
			((uint8_t *)j)[0] = 0;
		}
	}

	return elf.e_entry;
}

void load_game()
{
	uint32_t eip = load_elf(DISK_START);
	/* construct trapframe */
	TrapFrame tf;
	tf.eip = eip;
	tf.cs = 0x8;
	tf.eflags = 0x246;
	asm volatile("movl %0, %%esp;\
			pushl %1;\
			pushl %2;\
			pushl %3;\
			iret;"::"i"(GAME_STACK_ADDR), "r"(tf.eflags), "r"(tf.cs), "r"(tf.eip));
}
