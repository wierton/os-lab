#include "common.h"
#include "x86/x86.h"
#include "process/process.h"

#define USR_STACK_SIZE 4096


typedef struct {
	uint8_t stack[USR_STACK_SIZE];
	TrapFrame *tf;
} PCB;


