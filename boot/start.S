/* segment descriptor
 31          23             15             7           0
 +-----------+-+-+-+-+------+-+---+-+----+-+-----------+
 |           | | | |A|      | |   | |    | |           |
 |BASE 31..24|G|X|O|V|LIMIT |P|DPL|1|TYPE|A|BASE 23..16| 4
 |           | | | |L|19..16| |   | |    | |           |
 |-----------+-+-+-+-+------+-+---+-+----+-+-----------|
 |                          |                          |
 |   SEGMENT BASE 15..0     |    SEGMENT LIMIT 15..0   | 0
 |                          |                          |
 +--------------------------+--------------------------+
 */

#define MAKE_NULL_DESC \
	.word 0, 0;\
	.byte 0, 0, 0, 0

#define MAKE_SEG_DESC(type,base,lim)\
	.word ((lim) & 0xffff), ((base) & 0xffff);\
	.byte (((base) >> 16) & 0xff), (0x90 | (type)),\
	(0xC0 | (((lim) >> 16) & 0xf)), (((base) >> 24) & 0xff)

.code16
.global start
start:
	cli

	# zero segment register
	movw $0x0, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %ss

	# enter extra video mode
	movw $0x4f02, %ax
	movw $0x0112, %bx
	int $0x10

	# get information of extra video mode of 0x0115
	movw $0x2000, %di
	movw $0x4f01, %ax
	movw $0x0115, %cx
	int $0x10

	# open A20 address line, to access all memory
	movw $0x2401, %ax
	int $0x15
	
	lgdt gdtr
	movl %cr0, %eax		# enter protect mode by setting cr0
	orl	 $0x1, %eax
	movl %eax, %cr0

	# set CS(2 bits RPL, 1 bits TI and 13 bits index), 0x8 with index 0x1 which corresponding to the 2nd gdtdesc
	ljmp $0x8, $start_next

.code32
start_next:
	# refer to PA, the base addr of DS, ES, SS are set to zero
	movw $0x10, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %ss

	# set stack pointer
	movl $0x8000000, %esp
	movl $0x0, %ebp

	# enter c code
	call boot_main

.p2align 2
gdtdesc:
	MAKE_NULL_DESC
	MAKE_SEG_DESC(0xa, 0x0, 0xffffffff)		# code
	MAKE_SEG_DESC(0x2, 0x0, 0xffffffff)		# data

gdtr:
	.word (gdtr - gdtdesc - 1)	# limit
	.long gdtdesc				# base
