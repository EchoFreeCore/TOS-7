#ifndef TRAP_FRAME_H
#define TRAP_FRAME_H

#include <stdint.h>

typedef struct trap_frame {
    uint64_t regs[31];      // x0–x30
    uint64_t sp_el0;        // användarstack
    uint64_t elr_el1;       // return address
    uint64_t spsr_el1;      // saved PSTATE
    uint64_t esr_el1;       // exception syndrome
} trap_frame_t;

#endif