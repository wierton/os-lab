# init stack

#define STACK_SIZE (1 << 14)
#define va_to_pa(addr) (addr - 0xc0000000)

.global _start;
.global _stack_start_;
.global _stack_end_;
.global gdtdesc

_stack_start_:
	.space STACK_SIZE
_stack_end_:

_start:
	# init the stack top to _stack_end_
	movl $va_to_pa(_stack_end_), %esp
	pushl $va_to_pa(init_prev)
	ret

