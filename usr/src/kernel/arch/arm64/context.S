.global context_restore_and_eret
context_restore_and_eret :
mov x1, x0        // x0 = trap_frame*
ldp x0, x2, [x1, #(0 * 8)]
ldp x3, x4, [x1, #(2 * 8)]
ldp x5, x6, [x1, #(4 * 8)]
ldp x7, x8, [x1, #(6 * 8)]
ldp x9, x10, [x1, #(8 * 8)]
ldp x11, x12, [x1, #(10 * 8)]
ldp x13, x14, [x1, #(12 * 8)]
ldp x15, x16, [x1, #(14 * 8)]
ldp x17, x18, [x1, #(16 * 8)]
ldp x19, x20, [x1, #(18 * 8)]
ldp x21, x22, [x1, #(20 * 8)]
ldp x23, x24, [x1, #(22 * 8)]
ldp x25, x26, [x1, #(24 * 8)]
ldp x27, x28, [x1, #(26 * 8)]
ldp x29, x30, [x1, #(28 * 8)]

ldr x2, [x1, #(31 * 8)] // elr_el1
msr elr_el1, x2

ldr x2, [x1, #(32 * 8)] // spsr_el1
msr spsr_el1, x2

eret