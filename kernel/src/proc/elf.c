#include "common.h"
#include "math.h"
#include "x86/x86.h"
#include "x86/memory.h"
#include "device/fs.h"
#include "device/disk.h"
#include "proc/elf.h"
#include "proc/proc.h"

#define NR_MEMH 200

void set_memh(HANDLE hProc, uint32_t pm_num, MEMH *pm);

static MEMH memh[NR_MEMH];
static uint32_t nr_memh = 0;

uint32_t load_elf(HANDLE hProc, char *path)
{
	int i, j;
	uint32_t inodeno = opendir(path);
	INODE *pinode = open_inode(inodeno);

	Elf32_Endr elf;
	Elf32_Phdr ph[10];
	fs_read(pinode, 0, sizeof(Elf32_Endr), (void *)&elf);
	if(elf.e_phentsize != sizeof(Elf32_Phdr))
	{
		printk("error:open_inode\n");
		printk("inter no:%d\n", pinode->inodeno);
		printk("file:%s, no:%d, pointer:%x\n", path, inodeno, pinode);
		return 0;
	}
	assert(elf.e_phnum < 10 && elf.e_phentsize == sizeof(Elf32_Phdr));
	fs_read(pinode, elf.e_phoff, elf.e_phnum * elf.e_phentsize, ph);

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
		fs_read(pinode, ph[i].p_offset, ph[i].p_filesz, (void *)(ph[i].p_vaddr));
		for(j = ph[i].p_vaddr + ph[i].p_filesz; j < ph[i].p_vaddr + ph[i].p_memsz; j++)
		{
			((uint8_t *)j)[0] = 0;
		}
	}
	close_inode(pinode);

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
	HANDLE hGame = create_proc("/testcase", &pa);
	enter_proc(hGame);
}
