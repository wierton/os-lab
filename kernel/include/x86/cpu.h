#ifndef __CPU_H__
#define __CPU_H__

/* segment selector*/
typedef union {
	struct {
		uint32_t RPL    : 2;
		uint32_t TI     : 1;
		uint32_t INDEX  :13;
	};
	uint16_t val;
} R_SREG;

/* segment descriptor */
typedef struct SegmentDescriptor {
	uint32_t limit_15_0          : 16;
	uint32_t base_15_0           : 16;
	uint32_t base_23_16          : 8;
	uint32_t type                : 4;
	uint32_t segment_type        : 1;
	uint32_t privilege_level     : 2;
	uint32_t present             : 1;
	uint32_t limit_19_16         : 4;
	uint32_t soft_use            : 1;
	uint32_t operation_size      : 1;
	uint32_t pad0                : 1;
	uint32_t granularity         : 1;
	uint32_t base_31_24          : 8;
} SegDesc;

/* the 32bit Page Directory(first level page table) data structure */
typedef union PageDirectoryEntry {
	struct {
		uint32_t present             : 1;
		uint32_t read_write          : 1; 
		uint32_t user_supervisor     : 1;
		uint32_t page_write_through  : 1;
		uint32_t page_cache_disable  : 1;
		uint32_t accessed            : 1;
		uint32_t pad0                : 6;
		uint32_t page_frame          : 20;
	};
	uint32_t val;
} PDE;

/* the 32bit Page Table Entry(second level page table) data structure */
typedef union PageTableEntry {
	struct {
		uint32_t present             : 1;
		uint32_t read_write          : 1;
		uint32_t user_supervisor     : 1;
		uint32_t page_write_through  : 1;
		uint32_t page_cache_disable  : 1;
		uint32_t accessed            : 1;
		uint32_t dirty               : 1;
		uint32_t pad0                : 1;
		uint32_t global              : 1;
		uint32_t pad1                : 3;
		uint32_t page_frame          : 20;
	};
	uint32_t val;
} PTE;

/* to describe linear addr */
typedef union {
	struct {
		uint32_t off		:12;
		uint32_t pagetab    :10;
		uint32_t pagedir    :10;
	};
	uint32_t val;
} PageAddr;

/*segment register*/
typedef union {
	struct {
		uint32_t protect_enable      : 1;
		uint32_t monitor_coprocessor : 1;
		uint32_t emulation           : 1;
		uint32_t task_switched       : 1;
		uint32_t extension_type      : 1;
		uint32_t numeric_error       : 1;
		uint32_t pad0                : 10;
		uint32_t write_protect       : 1;
		uint32_t pad1                : 1;
		uint32_t alignment_mask      : 1;
		uint32_t pad2                : 10;
		uint32_t no_write_through    : 1;
		uint32_t cache_disable       : 1;
		uint32_t paging              : 1;
	};
	uint32_t val;
} CR0;

typedef union {
	struct {
		uint32_t pad0                : 3;
		uint32_t page_write_through  : 1;
		uint32_t page_cache_disable  : 1;
		uint32_t pad1                : 7;
		uint32_t page_directory_base : 20;
	};
	uint32_t val;
} CR3;
	
struct tagDTR {
	uint16_t limit;
	uint32_t base;
} GDTR,LDTR,IDTR;

typedef union{
	struct{
		unsigned CF:1;
		const unsigned :1;
		unsigned PF:1;
		const unsigned :1;
		unsigned AF:1;
		const unsigned :1;
		unsigned ZF:1;
		unsigned SF:1;
		unsigned TF:1;
		unsigned IF:1;
		unsigned DF:1;
		unsigned OF:1;
		unsigned OLIP:2;
		unsigned NT:1;
		const unsigned :1;
		unsigned RF:1;
		unsigned VM:1;
		const unsigned :14;
	};
	uint32_t val;
} EFLAGS;

typedef struct {
	uint32_t link;	// Old ts selector
	uint32_t esp0;	// Stack pointers and segment selectors
	uint16_t ss0;	//   after an increase in privilege level
	uint16_t padding1;
	uint32_t esp1;
	uint16_t ss1;
	uint16_t padding2;
	uint32_t esp2;
	uint16_t ss2;
	uint16_t padding3;
	uint32_t cr3;	// Page directory base
	uint32_t eip;	// Saved state from last task switch
	uint32_t eflags;
	uint32_t eax;	// More saved state (registers)
	uint32_t ecx;
	uint32_t edx;
	uint32_t ebx;
	uint32_t esp;
	uint32_t ebp;
	uint32_t esi;
	uint32_t edi;
	uint16_t es;	// Even more saved state (segment selectors)
	uint16_t padding4;
	uint16_t cs;
	uint16_t padding5;
	uint16_t ss;
	uint16_t padding6;
	uint16_t ds;
	uint16_t padding7;
	uint16_t fs;
	uint16_t padding8;
	uint16_t gs;
	uint16_t padding9;
	uint16_t ldt;
	uint16_t padding10;
	uint16_t t;		// Trap on task switch
	uint16_t iomb;	// I/O map base address
} Taskstate;

static inline uint32_t read_cr0()
{
	uint32_t val;
	asm volatile("movl %%cr0, %0":"=r"(val));
	return val;
}

static inline void write_cr0(uint32_t val)
{
	asm volatile("movl %0, %%cr0"::"r"(val));
}

static inline uint32_t read_cr3()
{
	uint32_t val;
	asm volatile("movl %%cr3, %0":"=r"(val));
	return val;
}

static inline void write_cr3(uint32_t val)
{
	asm volatile("movl %0, %%cr3"::"r"(val));
}

static inline void cli()
{
	asm volatile("cli");
}

static inline void sti()
{
	asm volatile("sti");
}

#endif
