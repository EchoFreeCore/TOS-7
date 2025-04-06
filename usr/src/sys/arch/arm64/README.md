# ARM64 Architecture Directory Overview

This directory implements the low-level ARM64 (AArch64) support for the **TOS-7** operating system kernel.  
It provides architecture-specific startup code, memory management, context switching, and trap/interrupt handling.

---

## Contents

| File         | Description |
|--------------|-------------|
| **boot.S**   | Early boot trampoline for multi-core startup. Sets up the CPU before MMU and high-level init is run. Executes in physical memory. |
| **context.S**| Implements `context_save()`, `context_restore()`, and `context_return()` for saving and restoring kernel thread state (`x19–x30 + SP`). Used by the scheduler. |
| **context.h**| Defines the `context_t` structure and interfaces for task switching and traps. |
| **cpu.c**    | CPU utility code: interrupt control, memory barriers, system register access, and privilege level transitions. |
| **cpu.h**    | Declares CPU-related operations: IRQ/FIQ masking, exception level switching, system registers, FPU control, cache/TLB management. |
| **link.ld**  | Linker script defining memory layout for the kernel image. Controls section placement in virtual and physical address space. |
| **mmu.S**    | Low-level MMU control: enables/disables MMU, sets TTBR registers, flushes TLBs. |
| **mmu.c**    | Page table allocator and virtual memory manager. Handles identity mapping, user space creation, ASID allocation, and COW fault logic. |
| **mmu.h**    | Public MMU API, page size constants, flags, and table walk utilities. |
| **ptdefs.h** | Page table definitions: shifts, masks, attributes, and page table level logic. |
| **start.S**  | Kernel startup entry point. Transitions from EL2→EL1, sets SP, initializes `.bss`, maps memory, enables MMU, sets up static base and TLS, then jumps to `main()`. |
| **start.h**  | Macros and declarations for use by `start.S`. |
| **trap.S**   | Full EL1 vector table. Handles sync/IRQ/FIQ/SError from both EL0 and EL1. Builds trapframes, supports syscalls (`vsys`) and returns via `eret`. |

---

## Functionality

The `arch/arm64` layer is responsible for:

- Switching from **EL2 → EL1** on cold boot
- Setting up the **MMU** and virtual memory mappings
- Defining **trap vectors** and syscall entry
- Building and restoring **trapframes** during exceptions
- Supporting **kernel thread context switches**
- Providing **atomic operations** and memory barriers
- Transitioning to **user mode (EL0)** execution via `eret`

This layer is designed to be **board-independent** and shared across ARM64 targets.  
Board-specific hardware setup (UART, GIC, memory layout) is expected to live under:
