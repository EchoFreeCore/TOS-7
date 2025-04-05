#include "start.h"
#include <stdint.h>
#include <stddef.h>

// ====================================
// Generic CPU Lifecycle & Privilege
// ====================================

void cpu_init(void) {
    _start();
}

void cpu_enter_privileged(void) {
    svcmode();
}

void cpu_enter_user(uint64_t sp_user, uint64_t entry_point) {
    touser();  // assume registers preloaded
}

void cpu_halt(void) {
    stop();
}


// =========================
// MMU (generic naming)
// =========================

void mmu_enable(void) {
    mmuenable();
}

void mmu_disable(void) {
    mmudisable();
}

void mmu_flush_tlb_local(void) {
    flushlocaltlb();
}

void mmu_flush_tlb_all(void) {
    flushtlb();
}

void mmu_flush_asid(uint64_t asid) {
    flushasid();
}

void mmu_flush_asid_va(uint64_t asid, void *va) {
    flushasidva();
}

void mmu_flush_asid_range(uint64_t asid) {
    flushasidvall();
}

void mmu_set_translation_base(uint64_t base) {
    setttbr(base);
}


// =========================
// Cache Control
// =========================

void cache_invalidate_icache(void) {
    cacheiinv();
}

void cache_clean_dcache(void) {
    cachedwbinv();
}

void cache_clean_l2(void) {
    l2cacheuwbinv();
}


// =========================
// FPU Control
// =========================

void fpu_enable(void) {
    fpon();
}

void fpu_disable(void) {
    fpoff();
}

void fpu_save_context(void *ctx) {
    fpsaveregs(ctx);
}

void fpu_restore_context(const void *ctx) {
    fploadregs((void*)ctx);
}


// =========================
// Interrupt Control
// =========================

void irq_disable(void) {
    splhi();
}

void irq_disable_all(void) {
    splfhi();
}

void irq_enable(void) {
    spllo();
}

void irq_enable_partial(void) {
    splflo();
}

void irq_restore(uint32_t flags) {
    splx(flags);
}

int irq_is_enabled(void) {
    return islo();
}


// =========================
// Atomics / Memory Sync
// =========================

int atomic_cmpxchg(uint32_t *addr, uint32_t expected, uint32_t desired) {
    return cas(addr, expected, desired);
}

int atomic_testset(uint32_t *addr) {
    return tas(addr);
}

void memory_barrier(void) {
    coherence();
}


// =========================
// Time / Counters
// =========================

uint64_t cpu_cycle_counter(void) {
    return lcycles();
}

uint64_t cpu_virtual_timer(void) {
    return vcycles();
}


// =========================
// Trap & Context
// =========================

int context_save(uintptr_t *label) {
    return setlabel(label);
}

int context_restore(uintptr_t *label) {
    return gotolabel(label);
}

void context_return(uintptr_t addr) {
    returnto(addr);
}

uint64_t fault_address(void) {
    return getfar();
}


// =========================
// Event primitives
// =========================

void cpu_event_signal(void) {
    sev();
}

void cpu_wait(void) {
    idlehands();
}


// =========================
// Fault-tolerant copy
// =========================

void safe_memcpy(const uint8_t *src, uint8_t *dst, uint32_t len) {
    peek(src, dst, len);
}


// =========================
// Hypervisor interface
// =========================

void hypervisor_call(void *trapframe) {
    hvccall(trapframe);
}


// =========================
// Debug / Dev-only
// =========================

void debug_loop(void) {
    aaa();
}


// =========================
// Platform-specific glue (ARM64)
// =========================

void arm64_vector_default(void)  { vtrap();  }
void arm64_vector_el0(void)      { vsys();   }
void arm64_vector_el0_sync(void) { vsys0();  }
void arm64_vector_el1_sync(void) { vtrap0(); }
void arm64_vector_el1_irq(void)  { virq();   }
void arm64_vector_el1_fiq(void)  { vfiq();   }
void arm64_vector_el1_serr(void) { vserr();  }
void arm64_trap_return(void)     { noteret(); }
void arm64_fork_return(void)     { forkret(); }
void arm64_trap_unknown(void)    { itsatrap(); }
