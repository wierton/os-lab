.global vec0;    vec0:   pushl $0;  pushl    $0; jmp do_irq
.global vec1;    vec1:   pushl $0;  pushl    $1; jmp do_irq
.global vec2;    vec2:   pushl $0;  pushl    $2; jmp do_irq
.global vec3;    vec3:   pushl $0;  pushl    $3; jmp do_irq
.global vec4;    vec4:   pushl $0;  pushl    $4; jmp do_irq
.global vec5;    vec5:   pushl $0;  pushl    $5; jmp do_irq
.global vec6;    vec6:   pushl $0;  pushl    $6; jmp do_irq
.global vec7;    vec7:   pushl $0;  pushl    $7; jmp do_irq
.global vec8;    vec8:              pushl    $8; jmp do_irq
.global vec9;    vec9:   pushl $0;  pushl    $9; jmp do_irq
.global vec10;   vec10:             pushl   $10; jmp do_irq
.global vec11;   vec11:             pushl   $11; jmp do_irq
.global vec12;   vec12:             pushl   $12; jmp do_irq
.global vec13;   vec13:             pushl   $13; jmp do_irq
.global vec14;   vec14:             pushl   $14; jmp do_irq

.global vecsys; vecsys:  pushl $0;  pushl $0x80; jmp do_irq

.global irq0;     irq0:  pushl $0;  pushl $1000; jmp do_irq
.global irq1;     irq1:  pushl $0;  pushl $1001; jmp do_irq
.global irq14;   irq14:  pushl $0;  pushl $1014; jmp do_irq
.global irq_empty;
			 irq_empty:	 pushl $0;  pushl	$-1; jmp do_irq

.extern cur_esp
.comm old_esp, 4, 4
.global do_irq
do_irq:
	pushl %ds;
	pushl %es;
	pushal;

	# switch to user's kernel stack
	movl %esp, %eax;
	movl %eax, old_esp;
	movl cur_esp, %esp;

	# call irq_handle
	pushl %eax;
	call irq_handle;
	addl $0x4, %esp;

	# switch back to user's stack
	movl old_esp, %esp;

	popal;
	addl $16, %esp;
	iret;

