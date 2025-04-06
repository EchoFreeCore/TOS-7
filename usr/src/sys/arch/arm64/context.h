#ifndef CONTEXT_H
#define CONTEXT_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// =======================================================
// context.h – Kernel context switching (EL1 ↔ EL1)
// -------------------------------------------------------
// Detta representerar tillräcklig CPU-state för att
// spara och återuppta en kernel-tråd (scheduler-context).
//
// Används av t.ex.:
//   - scheduler()
//   - yield()
//   - kthread_create()
// =======================================================

// Struktur som speglar sparade callee-saved register + SP
typedef struct context_t {
    uint64_t regs[12];  // x19–x30 (callee-saved)
    uint64_t sp;        // stack pointer
} context_t;

// =======================================================
// API
// =======================================================

// Spara nuvarande CPU-kontext i label (t.ex. proc->context)
// Returvärde:
//   - 0 första gången
//   - 1 när kontexten återställs med context_restore()
int context_save(context_t *ctx);

// Återställ en tidigare sparad kontext
// Returnerar aldrig – hoppar till plats där context_save lämnade
int context_restore(context_t *ctx);

// Direkt returnera till angiven PC (används efter fork, exec)
void context_return(uintptr_t entry_point);

// Läs faultad adress från senaste exception (FAR_EL1)
uint64_t fault_address(void);

#ifdef __cplusplus
}
#endif

#endif // CONTEXT_H
