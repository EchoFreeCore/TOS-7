.section.text
.global arch_mmu_write_ttbr0
arch_mmu_write_ttbr0 :
msr ttbr0_el1, x0
ret

.global arch_mmu_write_ttbr1
arch_mmu_write_ttbr1 :
msr ttbr1_el1, x0
ret

.global arch_mmu_setup
arch_mmu_setup :
// MAIR_EL1: attr 0 = normal, attr 1 = device
mov x0, #(0xff | (0x00 << 8))
msr mair_el1, x0

// TCR_EL1: 48-bit VA, 4KB granule, inner/outer WB WA
ldr x0, =0x00000000B5003518
msr tcr_el1, x0

// SCTLR_EL1: enable MMU + caches
mrs x0, sctlr_el1
orr x0, x0, #(1 << 0)   // M
orr x0, x0, #(1 << 2)   // C
orr x0, x0, #(1 << 12)  // I
msr sctlr_el1, x0

dsb sy
isb
ret