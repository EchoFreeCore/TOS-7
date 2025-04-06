// =======================================================
// context.S – Kernel context hantering för task switch
// -------------------------------------------------------
// Implementerar:
//   - context_save()
//   - context_restore()
//   - context_return()
//   - fault_address()
// 
// Layout: matchar struct context_t i context.h
// =======================================================

    .text
    .align 4

// === context_save: Sparar x19–x30 + SP ===
// x0 = pointer till context_t
.global context_save
.type context_save, %function
context_save:
    stp x19, x20, [x0, #(0*8)]
    stp x21, x22, [x0, #(2*8)]
    stp x23, x24, [x0, #(4*8)]
    stp x25, x26, [x0, #(6*8)]
    stp x27, x28, [x0, #(8*8)]
    stp x29, x30, [x0, #(10*8)]
    mov x1, sp
    str x1, [x0, #(12*8)]
    mov w0, #0
    ret

// === context_restore: Laddar x19–x30 + SP och återupptar ===
// x0 = pointer till context_t
.global context_restore
.type context_restore, %function
context_restore:
    ldp x19, x20, [x0, #(0*8)]
    ldp x21, x22, [x0, #(2*8)]
    ldp x23, x24, [x0, #(4*8)]
    ldp x25, x26, [x0, #(6*8)]
    ldp x27, x28, [x0, #(8*8)]
    ldp x29, x30, [x0, #(10*8)]
    ldr x1, [x0, #(12*8)]
    mov sp, x1
    mov w0, #1
    ret

// === context_return: Initiera ny körning från entry_point ===
// x0 = uintptr_t addr (PC att återgå till)
.global context_return
.type context_return, %function
context_return:
    msr elr_el1, x0
    mov x1, #0                // SPSR = EL0t, maskade IRQ (kan ändras)
    msr spsr_el1, x1
    eret

// === fault_address: returnera FAR_EL1 ===
.global fault_address
.type fault_address, %function
fault_address:
    mrs x0, FAR_EL1
    ret