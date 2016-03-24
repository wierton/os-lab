#ifndef __CPU_H__
#define __CPU_H__

static inline uint32_t read_cr0()
{
	uint32_t val;
	asm volatile("movl %cr0, %0":"=a"(val));
	return val;
}

static inline void write_cr0(uint32_t val)
{
	asm volatile("movl %0, %cr0"::"a"(val));
}

static inline uint32_t read_cr3()
{
	uint32_t val;
	asm volatile("movl %cr3, %0":"=a"(val));
	return val;
}

static inline void write_cr3(uint32_t val)
{
	asm volatile("movl %0, %cr3"::"a"(val));
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
