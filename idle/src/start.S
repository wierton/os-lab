# call testcase and quit by syscall

.extern main
.global _start

_start:
	call main
	movl $4, %eax
	int $0x80
