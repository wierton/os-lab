#include "common.h"
#include "x86/x86.h"
#include "x86/memory.h"
#include "device/disk.h"
#include "proc/elf.h"
#include "proc/proc.h"

#define NR_MEMH 200

void set_memh(HANDLE hProc, uint32_t pm_num, MEMH *pm);

static MEMH memh[NR_MEMH];
static uint32_t nr_memh = 0;

uint32_t load_elf(HANDLE hProc, uint32_t disk_start)
{
	int i, j;
	Elf32_Endr elf;
	Elf32_Phdr ph[10];
	read_disk(((uint32_t)&elf), disk_start, sizeof(Elf32_Endr));
	assert(elf.e_phnum < 10 && elf.e_phentsize == sizeof(Elf32_Phdr));
	read_disk(((uint32_t)ph), disk_start + elf.e_phoff, elf.e_phnum * elf.e_phentsize);

	mm_alloc(hProc, USER_STACK_ADDR - USER_STACK_SIZE, USER_STACK_SIZE);

	/* load into memory */
	for(i = 0; i < elf.e_phnum; i++)
	{
		memh[i + nr_memh] = make_memh(ph[i].p_vaddr, ph[i].p_paddr, ph[i].p_memsz);
		mm_alloc(hProc, ph[i].p_vaddr, ph[i].p_memsz);
		for(j = ph[i].p_vaddr; j < ph[i].p_vaddr + ph[i].p_memsz; j += 4)
		{
			/* make sure it can be read and write */
			((uint32_t *)j)[0] = j;
		}
	}

	set_memh(hProc, elf.e_phnum, &memh[nr_memh]);
	nr_memh += elf.e_phnum;
	assert(nr_memh < NR_MEMH);

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

void set_usrtf(uint32_t eip, TrapFrame *tf)
{
	/* pop */
	tf->ds = 0x20 | 0x3;
	tf->es = 0x20 | 0x3;
	/* iret */
	tf->eflags = 0x246;
	tf->cs = 0x18 | 0x3;
	tf->eip = eip;
	/* cross rings */
	tf->esp = USER_STACK_ADDR;
	tf->ss = 0x20 | 0x3;
}

void env_run(TrapFrame *tf)
{
	asm volatile("movl %0, %%esp;"
			"popal;"
			"popl %%es;"
			"popl %%ds;"
			"addl $8, %%esp;"
			"iret;"::"r"(tf));
}

void load_game()
{
	ProcAttr pa = {3};
	HANDLE hGame = create_proc(DISK_START, &pa);
	enter_proc(hGame);
}
