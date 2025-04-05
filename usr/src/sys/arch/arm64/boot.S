// boot.S — AArch64 UNIX kernel entry, för ett nyfött system

.section .text
.global _start
.type _start, %function

_start:
    // === Init stack ===
    ldr x0, =_stack_top
    mov sp, x0

    // === Disable MMU, Caches ===
    mrs x1, sctlr_el1
    bic x1, x1, #(1 << 0)   // M (MMU disable)
    bic x1, x1, #(1 << 2)   // C (Data cache)
    bic x1, x1, #(1 << 12)  // I (Instruction cache)
    msr sctlr_el1, x1
    isb

    // === Zero BSS ===
    ldr x0, =__bss_start
    ldr x1, =__bss_end
1:
    cmp x0, x1
    b.ge 2f
    str xzr, [x0], #8
    b 1b
2:

    // === Set trap vector base ===
    ldr x0, =_start_vectors
    msr VBAR_EL1, x0
    isb

    // === Enable IRQ/FIQ ===
    msr DAIFClr, #0xf

    // === Jump into C world ===
    bl kernel_main

.hang:
    wfe         // Idle forever if kernel_main returns
    b .hang

// =====================================================
// Stack (8 KB, aligned)
// =====================================================
.section .bss
.align 12
.global _stack
_stack:
    .skip 8192
.global _stack_top
_stack_top:

// =====================================================
// Symbols from linker script
// =====================================================
.extern __bss_start
.extern __bss_end
.extern _start_vectors
.extern kernel_main
