#ifndef ARCH_ARM64_MMU_H
#define ARCH_ARM64_MMU_H

#include <stdint.h>

void arch_mmu_write_ttbr0(uint64_t val);
void arch_mmu_write_ttbr1(uint64_t val);
void arch_mmu_setup(void); // skriver TCR, MAIR, SCTLR, etc.

#endif