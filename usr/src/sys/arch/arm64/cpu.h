#ifndef CPUWRAP_H
#define CPUWRAP_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// ====================================
// Generic CPU Lifecycle & Privilege
// ====================================
void cpu_init(void);
void cpu_halt(void);
void cpu_enter_privileged(void);
void cpu_enter_user(uint64_t user_sp, uint64_t entry_pc);
int  this_cpu(void);

// ====================================
// MMU Control
// ====================================
void mmu_enable(void);
void mmu_disable(void);
void mmu_flush_tlb_local(void);
void mmu_flush_tlb_all(void);
void mmu_flush_asid(uint64_t asid);
void mmu_flush_asid_va(uint64_t asid, void *va);
void mmu_flush_asid_range(uint64_t asid);
void mmu_set_translation_base(uint64_t ttbr);

// ====================================
// Cache Control
// ====================================
void cache_invalidate_icache(void);
void cache_clean_dcache(void);
void cache_clean_l2(void);

// ====================================
// FPU (SIMD/VFP)
// ====================================
void fpu_enable(void);
void fpu_disable(void);
void fpu_save_context(void *ctx);
void fpu_restore_context(const void *ctx);

// ====================================
// Interrupt Control
// ====================================
void irq_disable(void);
void irq_disable_all(void);
void irq_enable(void);
void irq_enable_partial(void);
void irq_restore(uint32_t flags);
int  irq_is_enabled(void);

// ====================================
// Atomics & Memory Sync
// ====================================
int  atomic_cmpxchg(uint32_t *addr, uint32_t expected, uint32_t desired);
int  atomic_testset(uint32_t *addr);
void memory_barrier(void);

// ====================================
// Time / Counters
// ====================================
uint64_t cpu_cycle_counter(void);       // physical/core cycles
uint64_t cpu_virtual_timer(void);       // monotonic virtual timer

// ====================================
// Event primitives
// ====================================
void cpu_event_signal(void);    // SEV
void cpu_wait(void);            // WFI / WFE

// ====================================
// Fault-tolerant Memory Copy
// ====================================
void safe_memcpy(const uint8_t *src, uint8_t *dst, uint32_t len);

// ====================================
// Hypervisor / Secure Monitor
// ====================================
void hypervisor_call(void *trapframe);

// ====================================
// Debug / Diagnostics
// ====================================
void debug_loop(void);          // spin + output

// ====================================
// ARM64-specific vector handlers
// ====================================
void arm64_vector_default(void);     // vtrap
void arm64_vector_el0(void);         // vsys
void arm64_vector_el0_sync(void);    // vsys0
void arm64_vector_el1_sync(void);    // vtrap0
void arm64_vector_el1_irq(void);     // virq
void arm64_vector_el1_fiq(void);     // vfiq
void arm64_vector_el1_serr(void);    // vserr
void arm64_trap_return(void);        // noteret
void arm64_fork_return(void);        // forkret
void arm64_trap_unknown(void);       // itsatrap

#ifdef __cplusplus
}
#endif

#endif // CPUWRAP_H
