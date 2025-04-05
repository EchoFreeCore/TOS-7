// trap.S – AArch64 full trap vector (EL1, SP_EL1)

.global _start_vectors
.align 11

// =======================================================
// Vector table layout (per AArch64 ARM DDI 0487)
// Offsets: 0x000–0x380 (sync/irq/fiq/serr from EL0/EL1)
// =======================================================

_start_vectors:
    b handle_sync_el0       // 0x000
    b handle_irq_el0        // 0x080
    b handle_fiq_el0        // 0x100
    b handle_serror_el0     // 0x180

    b handle_sync_el1       // 0x200
    b handle_irq_el1        // 0x280
    b handle_fiq_el1        // 0x300
    b handle_serror_el1     // 0x380

// =======================================================
// 0x000: Synchronous trap from EL0 (syscall, etc)
// =======================================================
.global handle_sync_el0
handle_sync_el0:
    sub sp, sp, #(34*8)
    mov x1, sp

    stp x0,  x1,  [x1, #( 0*8)]
    stp x2,  x3,  [x1, #( 2*8)]
    stp x4,  x5,  [x1, #( 4*8)]
    stp x6,  x7,  [x1, #( 6*8)]
    stp x8,  x9,  [x1, #( 8*8)]
    stp x10, x11, [x1, #(10*8)]
    stp x12, x13, [x1, #(12*8)]
    stp x14, x15, [x1, #(14*8)]
    stp x16, x17, [x1, #(16*8)]
    stp x18, x19, [x1, #(18*8)]
    stp x20, x21, [x1, #(20*8)]
    stp x22, x23, [x1, #(22*8)]
    stp x24, x25, [x1, #(24*8)]
    stp x26, x27, [x1, #(26*8)]
    stp x28, x29, [x1, #(28*8)]
    str x30,       [x1, #(30*8)]  // LR

    mrs x2, sp_el0
    str x2, [x1, #(31*8)]
    mrs x2, elr_el1
    str x2, [x1, #(32*8)]
    mrs x2, spsr_el1
    str x2, [x1, #(33*8)]
    mrs x2, esr_el1
    str x2, [x1, #(34*8)]

    mov x0, x1
    bl trap_handle

    mov x1, sp
    ldp x0,  x2,  [x1, #( 0*8)]
    ldp x3,  x4,  [x1, #( 2*8)]
    ldp x5,  x6,  [x1, #( 4*8)]
    ldp x7,  x8,  [x1, #( 6*8)]
    ldp x9,  x10, [x1, #( 8*8)]
    ldp x11, x12, [x1, #(10*8)]
    ldp x13, x14, [x1, #(12*8)]
    ldp x15, x16, [x1, #(14*8)]
    ldp x17, x18, [x1, #(16*8)]
    ldp x19, x20, [x1, #(18*8)]
    ldp x21, x22, [x1, #(20*8)]
    ldp x23, x24, [x1, #(22*8)]
    ldp x25, x26, [x1, #(24*8)]
    ldp x27, x28, [x1, #(26*8)]
    ldp x29, x30, [x1, #(28*8)]

    ldr x2, [x1, #(32*8)]
    msr elr_el1, x2
    ldr x2, [x1, #(33*8)]
    msr spsr_el1, x2

    add sp, sp, #(34*8)
    eret

// =======================================================
// IRQ handler from EL1
// =======================================================
.global handle_irq_el1
handle_irq_el1:
    sub sp, sp, #16
    stp x0, x1, [sp]
    bl irq_handle
    ldp x0, x1, [sp]
    add sp, sp, #16
    eret

// =======================================================
// FIQ / SError from EL1 – stubs
// =======================================================
.global handle_fiq_el1
handle_fiq_el1:
    b handle_irq_el1

.global handle_serror_el1
handle_serror_el1:
    b handle_irq_el1

// =======================================================
// Forward EL0 traps to EL1 handlers
// =======================================================
.global handle_irq_el0
handle_irq_el0:
    b handle_irq_el1

.global handle_fiq_el0
handle_fiq_el0:
    b handle_fiq_el1

.global handle_serror_el0
handle_serror_el0:
    b handle_serror_el1

.global handle_sync_el1
handle_sync_el1:
    b handle_irq_el1
