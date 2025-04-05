#ifndef TRAP_H
#define TRAP_H

#include "regs.h"

typedef enum {
    TRAP_SYSCALL,
    TRAP_PAGEFAULT,
    TRAP_UNKNOWN
} trap_reason_t;

void trap_handle(trapframe_t *tf);
void irq_handle(trapframe_t *tf);

// Implementeras av arkitekturspecifik kod:
trap_reason_t arch_decode_trap(trapframe_t *tf);
uint64_t arch_get_fault_addr(void);

#endif
