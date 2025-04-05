// start.h - ARM64 Kernel Boot + MMU Setup Interface
// ================================================
// Denna header exponerar konstanter och externa symboler definierade i start.S
// som används i kernel-start, MMU-setup, trapramverk och kontexthantering.
//
// Den är avsedd att inkluderas i C-kod i operativsystemets kärna (Plan 9, bare metal etc.)

#ifndef START_H
#define START_H

#include <stdint.h>

#ifdef __ASSEMBLER__
// Assembly-only makron

// Fysisk bas för virtuell mappning
.equ KZERO,         0xffff000000000000

// MMU och cache enable-bitar i SCTLR
.equ SCTLR_MMU,     (1 << 12) | (1 << 2) | (1 << 0)  // I, C, M

// MAIR (Memory Attributes)
.equ MA_DEV_nGnRnE, 0
.equ MA_DEV_nGnRE,  1
.equ MA_DEV_nGRE,   2
.equ MA_DEV_GRE,    3
.equ MA_MEM_UC,     4
.equ MA_MEM_WT,     5
.equ MA_MEM_WB,     6

.equ MAIR_INIT,     (0xFF << (MA_MEM_WB * 8)) | \
                    (0x33 << (MA_MEM_WT * 8)) | \
                    (0x44 << (MA_MEM_UC * 8)) | \
                    (0x00 << (MA_DEV_nGnRnE * 8)) | \
                    (0x04 << (MA_DEV_nGnRE * 8)) | \
                    (0x08 << (MA_DEV_nGRE * 8)) | \
                    (0x0C << (MA_DEV_GRE * 8))

// Translation Control Register
.equ SHARE_INNER,   3
.equ CACHE_WB,      1
.equ PGSHIFT,       12
.equ EVASHIFT,      39

.equ TCR_INIT, \
    (0 << 38) | (0 << 37) | (0 << 36) | \
    ((((3 << 16) | (1 << 14) | (2 << 12)) >> PGSHIFT) & 3) << 30 | \
    (SHARE_INNER << 28) | (CACHE_WB << 26) | (CACHE_WB << 24) | \
    (0 << 23) | (0 << 22) | ((64 - EVASHIFT) << 16) | \
    ((((1 << 16) | (2 << 14) | (0 << 12)) >> PGSHIFT) & 3) << 14 | \
    (SHARE_INNER << 12) | (CACHE_WB << 10) | (CACHE_WB << 8) | \
    (0 << 7) | ((64 - EVASHIFT) << 0)

// SCTLR initmasker
.equ SCTLR_CLR, \
    (3 << 30) | (1 << 27) | (1 << 26) | (1 << 25) | \
    (1 << 24) | (1 << 21) | (1 << 19) | (1 << 18) | \
    (1 << 17) | (1 << 16) | (1 << 15) | (1 << 14) | \
    (1 << 13) | (1 << 10) | (1 << 9)  | (1 << 4)  | \
    (1 << 3)  | (1 << 1)

.equ SCTLR_SET, \
    (3 << 28) | (3 << 22) | (1 << 20) | (1 << 11)

#else
// För C-kod

#ifdef __cplusplus
extern "C" {
#endif

// ================================================
// Externa symboler från start.S
// ================================================

// Boot & setup
extern void _start(void);
extern void svcmode(void);
extern void touser(void);
extern void stop(void);

// MMU / Cache
extern void mmuenable(void);
extern void mmudisable(void);
extern void flushlocaltlb(void);
extern void flushtlb(void);
extern void flushasid(void);
extern void flushasidva(void);
extern void flushasidvall(void);
extern void cacheiinv(void);
extern void cachedwbinv(void);
extern void l2cacheuwbinv(void);

// FP (SIMD/VFP)
extern void fpon(void);
extern void fpoff(void);
extern void fploadregs(void*);
extern void fpsaveregs(void*);

// Trap / Exception vectors
extern void vtrap(void);
extern void vtrap0(void);
extern void vtrap1(void);
extern void vfiq(void);
extern void virq(void);
extern void vserr(void);
extern void vsys(void);
extern void vsys0(void);
extern void noteret(void);
extern void forkret(void);
extern void itsatrap(void);

// Syscall interface
extern void main(uint64_t);

// Concurrency / Atomics
extern int cas(uint32_t *addr, uint32_t ov, uint32_t nv);
extern int tas(uint32_t *addr);
extern void coherence(void);

// Interrupts
extern int islo(void);
extern int splhi(void);
extern int splfhi(void);
extern void spllo(void);
extern void splflo(void);
extern void splx(uint32_t);

// Time
extern uint64_t vcycles(void);
extern uint64_t lcycles(void);

// Trapframe manipulation
extern int setlabel(uintptr_t*);
extern int gotolabel(uintptr_t*);
extern void returnto(uintptr_t);
extern uint64_t getfar(void);
extern void setttbr(uint64_t);

// Wait-for/Signal primitives
extern void sev(void);
extern void idlehands(void);

// Memory copying (fault-tolerant)
extern void peek(const uint8_t *src, uint8_t *dst, uint32_t len);

// Hypervisor
extern void hvccall(void *ureg);

// Debug/testing
extern void aaa(void);

#ifdef __cplusplus
}
#endif

#endif  // __ASSEMBLER__

#endif // START_H
