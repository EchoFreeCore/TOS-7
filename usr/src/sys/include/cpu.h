#include <stdint.h>
#include "cpu.h"

int this_cpu(void) {
    uint64_t mpidr;
    asm volatile("mrs %0, mpidr_el1" : "=r"(mpidr));
    return mpidr & 0b11; // Core ID (0–3)
}

#ifndef ARCH_CPU_H
#define ARCH_CPU_H

int this_cpu(void);  // Returnerar logisk CPU-ID

#endif
