#ifndef TRAP_FRAME_H
#define TRAP_FRAME_H

#include <stdint.h>

// ==========================================================
// Trap frame layout för AArch64 EL0 → EL1 övergång
// Matchar exakt layouten i trap.S
// Storlek: 35 * 8 = 280 bytes
// ==========================================================

typedef struct trap_frame {
    uint64_t regs[31];      // x0–x30 (allmänna register)
    uint64_t sp_el0;        // användarstack vid trap
    uint64_t elr_el1;       // return address till EL0
    uint64_t spsr_el1;      // sparad PSTATE
    uint64_t esr_el1;       // exception syndrome
} trap_frame_t;

#define TF_X(n)         ((n) * 8)
#define TF_SP_EL0       (31 * 8)
#define TF_ELR_EL1      (32 * 8)
#define TF_SPSR_EL1     (33 * 8)
#define TF_ESR_EL1      (34 * 8)
#define TF_SIZE         (35 * 8)  // 280 bytes

#endif // TRAP_FRAME_H
