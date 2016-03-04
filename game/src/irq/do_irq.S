.global vec0; vec0: pushl $0; jmp do_irq;
.global vec1; vec1: pushl $1; jmp do_irq;
.global vec2; vec2: pushl $2; jmp do_irq;
.global vec3; vec3: pushl $3; jmp do_irq;
.global vec4; vec4: pushl $4; jmp do_irq;
.global vec5; vec5: pushl $5; jmp do_irq;
.global vec6; vec6: pushl $6; jmp do_irq;
.global vec7; vec7: pushl $7; jmp do_irq;
.global vec8; vec8: pushl $8; jmp do_irq;
.global vec9; vec9: pushl $9; jmp do_irq;
.global vec10; vec10: pushl $10; jmp do_irq;
.global vec11; vec11: pushl $11; jmp do_irq;
.global vec12; vec12: pushl $12; jmp do_irq;
.global vec13; vec13: pushl $13; jmp do_irq;

.global irq0; irq0: pushl $1000; jmp do_irq; # timer
.global irq1; irq1: pushl $1001; jmp do_irq; # keyboard

.global irq_empty; irq_empty: pushl $-1; jmp do_irq; # unhandled

.global do_irq
do_irq:
	pushal;
	pushl %esp;
	call irq_handle;
	addl $0x4, %esp;
	popal;
	addl $0x4, %esp;
	iret;

