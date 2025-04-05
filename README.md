# TOS/7 
 
TOS/7 Embedded System
POSIX.1-1988

Userland in TOS/7 is not an ecosystem.
It is a toolkit.

A set of static tools — understandable, maintainable, rebuildable.

It is UNIX as it was meant to be:

A small number of well-crafted programs that work together,
speak via file descriptors, and don’t overstep.

--------------------------
OVERVIEW
--------------------------

This document outlines the staged release progression of TOS/7, a minimal
UNIX-inspired operating system with a focus on clarity, static linkage, 
manual simplicity, and research-grade transparency.

TOS/7 is not a clone, nor a reimplementation — it is a new UNIX-like 
kernel and userland created in the spirit of V6, V7, and Research V10, 
reborn with modern toolchains and retro engineering intent.

The system is composed of the following primary components:

    - Trap and Interrupt Handling
    - Syscall Dispatch
    - Process and Thread Scheduling
    - Virtual Memory Management
    - File System Interface (VFS)
    - Device I/O
    - Initialization and Boot Procedure

All components are written in ANSI C and GAS-style assembly.

NOTES:
- All modules are written in C89-compatible style.
- Assembly follows GAS syntax, minimal and readable.
- No dynamic linking. No kernel modules. No config files.
- If it cannot be explained on a whiteboard, it is not here.

TOS/7 is a reaction to complexity.
It is a statement that clarity still matters.

--------------------------
SYSCALL DISPATCH
--------------------------

Syscalls are listed in `syscalls.tbl`. The system uses a generated dispatcher
in `syscall.c`, routing based on syscall number.

Each syscall is implemented in a dedicated subsystem module (e.g. `vfs.c`,
`scheduler.c`, `mmu.c`).

The syscall ABI is modeled after FreeBSD for libc compatibility, but only
a minimal subset is implemented for early bootstrapping.

--------------------------
TRAP AND INTERRUPTS
--------------------------

Trap handling is initialized early in `start.S` and configured via `VBAR_EL1`.
Traps are dispatched through `trap.c`, which decodes exception types and
routes accordingly.

System calls are issued via `SVC #0` (AArch64) and enter at `trap_handler()`.

Interrupts (IRQ) are routed to `irq_handler()` and primarily service the
system timer (scheduler tick).

--------------------------
MEMORY MANAGEMENT
--------------------------

TOS/7 uses a page-table based MMU design.

    - Kernel heap is allocated via `kmalloc.c`
    - User space memory via `brk()` and `mmap()`
    - Physical frames and mappings in `paging.c`

--------------------------
FILE SYSTEM AND VFS
--------------------------

The VFS layer is implemented in `vfs.c`. It manages file descriptors and
dispatches to underlying file systems or devices.

The default file system is a EXT2 FS implemented.

Inodes, directory entries, and block I/O are handled explicitly, with no
dynamically loaded modules.

Devices are registered via `devfs.c` and exposed through `/dev`.

--------------------------
PROCESS MANAGEMENT
--------------------------

Process management includes PID allocation, fork/exec semantics, and
exit/wait handling.

Processes are built around kernel threads and isolated address spaces.

The ELF loader (`exec.c`) maps a user binary and sets up its initial stack.

--------------------------
DEVICE I/O
--------------------------

Device drivers are minimal, character-based, and statically registered.

The system supports:

    - Serial TTY via `uart.c`
    - Console logging via `console.c`
    - Input line buffers via `input.c`
    - Pipes via `pipe.c`

All device I/O is synchronous and buffered within the kernel

--------------------------
SCHEDULER
--------------------------

TOS/7 supports preemptive multithreading, using a fixed-priority, multi-class
scheduler.

There are three classes:

    - SCHED_SYS: system and idle threads
    - SCHED_TS: time-sharing user threads
    - SCHED_RT: fixed-priority real-time threads

Context switches are performed via `context_switch()` in assembly.
Sleep queues and timeouts are handled in `sleep.c` and `timer.c`.

--------------------------
BOOT PROCESS
--------------------------

The kernel entry point is `_start` in `start.S`. It performs:

    - Initial stack setup
    - MMU and trap vector configuration
    - Jump to `kernel_main()` in `boot.c`

`kernel_main()` performs system initialization:

    - Initialize heap, scheduler, MMU, and drivers
    - Mount root filesystem
    - Spawn `/bin/init`

`init` launches `/bin/sh` or similar minimal userland.


--------------------------
USERLAND INTERFACE
--------------------------

The system expects statically linked ELF binaries.

Only a minimal libc is required to call into the kernel via syscalls.
No dynamic linking, daemons, or services are needed.

--------------------------
FUTURE WORK
--------------------------

    - Implement `execve()` with full argv/envp support
    - Add `/proc`, `/sys`, and virtual filesystems
    - Support pthread-style threading
    - Implement signal delivery and timers
    - Write a full libc to match kernel ABI

--------------------------
RELEASE STAGES
--------------------------

Each version increment introduces well-scoped capabilities and preserves 
system stability. Simplicity precedes feature expansion. Stability is 
preferred over abstraction.

---

VERSION: TOS/7
Status:   Active Development (Pre-release)
Goal:     Static UNIX Kernel with POSIX Core

Features:
  • Static ELF kernel and userland
  • Minimal system calls: execve, fork, waitpid, exit, open, read, write
  • Flat VFS (ext2), block driver, and basic inode I/O
  • UART/console I/O with TTY driver
  • Simple preemptive scheduler with tick-based time slicing
  • init process + rc script
  • Shell, ls, cat, echo, uptime
  • Manual memory allocation (brk/sbrk)

Marker:
  /etc/tos-release → "TOS/7 (2025) — Static Core"

---

VERSION: TOS/7
Status:   Stable Core (Future milestone)
Goal:     POSIX Kernel Subset with Userland

Additions:
  • Expanded syscall ABI (FreeBSD alignment)
  • Thread syscalls (210–217)
  • Kernel memory zones, kmalloc
  • Directory operations, rename, mkdir, unlink
  • Improved stat(), time, sleep()
  • Sh, cp, mv, rm, ps, kill, mount
  • libc v1 and stdio core support
  • Manpages, cron, and /etc structure

---

VERSION: TOS/8
Status:   Next Generation System (Optional)
Goal:     Dynamic Linking, Signals, and Paging

Additions:
  • Shared object support (ld.so)
  • Virtual memory with page faults
  • Copy-on-write fork
  • Signal delivery, sigaction(), sigmask()
  • libc.so with modular headers
  • File-backed mmap(), PROT flags

---

VERSION: TOS-9
Status:   Visionary Milestone
Goal:     SMP, networking, packages

Additions:
  • SMP scheduler with ASID and TLB isolation
  • Network stack with loopback + device driver
  • Package manager (pkg(8)-like)
  • Dynamic framebuffer/VGA
  • useradd, login, tty, init multi-user mode
  • Build system redesign (optional make replacement)

--------------------------
VERSIONING POLICY
--------------------------

• TOS/7: development and research track  
• TOS/8: stable ABI release for userland compatibility  
• TOS/9 and beyond: maintain minimalism, introduce capability incrementally  
• Syscall table is append-only, compatible with FreeBSD ABI numbers

--------------------------
PHILOSOPHY
--------------------------

The release system is not a sprint but a meditation.

It is built for those who believe code should be read, understood,
and trusted. Every syscall has a purpose. Every subsystem has an owner.
There are no magic services. There is only UNIX.

Each milestone is a shape in time — not an obligation to modernity.
We build what is understandable. What is stable is beautiful.
What is simple is enduring.

--------------------------
END OF RELEASE_PLAN.txt



TOS/7 KERNEL — TODO LIST
-------------------------
Appendix G: "Future Work"
Document version: April 1983 (or so we pretend)
System: TOS/7 0.1-ALPHA
Philosophy: Simplicity is the ultimate sophistication.

-------------------------
1. PROCESSING AND SMP
-------------------------

- [ ] Implement per-CPU scheduler queues
- [ ] Add CPU-local data areas
- [ ] Implement `smp_init()` and detect secondary cores
- [ ] Use `ipi_send()` to trigger inter-CPU signals
- [ ] Add load balancing between CPUs
- [ ] Add thread affinity and CPU pinning
- [ ] Lockless scheduler queues for SCHED_RT

-------------------------
2. MEMORY MANAGEMENT
-------------------------

- [ ] Write complete `mmu.c` with full page table abstraction
- [ ] Support demand-paging and COW for `fork()`
- [ ] Add simple `vma.c` (Virtual Memory Area tracking)
- [ ] Support `mmap()` for file-backed mappings
- [ ] Write minimal page frame allocator (buddy or bitmap)
- [ ] Add `/proc/self/maps` for debugging memory layout

-------------------------
3. FILE SYSTEM
-------------------------

- [ ] Add write caching and inode journaling (optional)
- [ ] Implement `fsck` tool for recovery
- [ ] Support mounting root FS from block device
- [ ] Build a `/dev` device node manager
- [ ] Add VFS notifications for open/close/read/write

-------------------------
4. SYSTEM CALLS
-------------------------

- [ ] Implement full `execve()` with argv/envp
- [ ] Add `select()` and `poll()` for I/O multiplexing
- [ ] Implement `ioctl()` for basic terminal settings
- [ ] Implement proper `sigaction()` and signal delivery
- [ ] Add `reboot()` with clean shutdown hooks
- [ ] Track syscall usage and profiling stats

-------------------------
5. USERLAND INTERFACE
-------------------------

- [ ] Write a Next Generation `libc` (libtos) for 2025:
      - POSIX ABI, static link, syscall-only
      - Manual page layout (syscall/syscall.c)
      - Optional libc replacement for minimal embedded use

- [ ] Write `/bin/sh` as a non-forking shell
- [ ] Add `/bin/ps`, `/bin/ls`, `/bin/kill`, `/bin/mount`
- [ ] Implement basic `/dev/tty`, `/dev/null`, `/dev/zero`
- [ ] Add `/proc/<pid>/status` and `/proc/self/*`

-------------------------
6. TOOLCHAIN AND BUILD
-------------------------

- [ ] Fully static toolchain: `musl`-free, glibc-free
- [ ] Simple `Makefile`, no `configure`, no autotools
- [ ] Generate syscall headers from `syscalls.tbl`
- [ ] ELF loader that can parse modern static binaries

-------------------------
7. DOCUMENTATION
-------------------------

- [ ] Manual pages in Markdown + ASCII
- [ ] `man 2 write` should work from `/bin/man`
- [ ] Include architecture diagrams (`ARCHITECTURE.md`)
- [ ] Cross-reference to UNIX v7/Lions when applicable

-------------------------
8. CLEANUP AND CONSISTENCY
-------------------------

- [ ] Ensure all modules use `panic()` for fatal faults
- [ ] Audit memory allocation for leaks
- [ ] Make all trap handlers reentrant
- [ ] Use consistent naming (`sys_`, `vfs_`, `proc_`)

-------------------------
9. IDEAS UNDER CONSIDERATION
-------------------------

- [ ] Add support for user-level threading (libuthread)
- [ ] Remote TTY over serial (like early Unix terminals)
- [ ] `ptrace()`-like minimal debugger interface
- [ ] Time-traveling debugger like rr (just dreaming...)
- [ ] Rebuild UNIX v7 userland on top of TOS/7 (for science)

-------------------------
FINAL NOTE
-------------------------

TOS/7 is not a clone of UNIX.
It is UNIX in spirit — minimal, clean, readable, and buildable
by one person with a text editor and a dream.

We do not add features.
We remove everything that is not essential to control.

-- End of TODO.txt --


/
├── boot/
│   ├── kernel.img
│   └── start.elf
├── dev/
│   └── null.c
├── etc/
│   ├── passwd
│   ├── ttytab
│   └── rc
├── lib/
│   └── libc.a
├── proc/
├── sbin/
│   ├── fsck
│   └── mount
├── tmp/
├── usr/
│   ├── bin/
│   │   ├── make
│   │   └── cc
│   ├── include/
│   │   ├── assert.h
│   │   ├── ctype.h
│   │   ├── errno.h
│   │   ├── fcntl.h
│   │   ├── limits.h
│   │   ├── setjmp.h
│   │   ├── signal.h
│   │   ├── stdarg.h
│   │   ├── stddef.h
│   │   ├── stdio.h
│   │   ├── stdlib.h
│   │   ├── string.h
│   │   ├── syscall.h
│   │   ├── syscalls.tbl
│   │   ├── sys/
│   │   │   ├── mman.h
│   │   │   ├── syscall.h
│   │   │   └── types.h
│   │   ├── time.h
│   │   └── unistd.h
│   ├── lib/
│   │   ├── libm.a
│   │   ├── libc.a
│   │   └── libc/
│   │       ├── assert/assert.c
│   │       ├── ctype/ctype.c
│   │       ├── errno/errno.c
│   │       ├── fcntl/fcntl.c
│   │       ├── limits/limits.c
│   │       ├── setjmp/setjmp.c
│   │       ├── signal/signal.c
│   │       ├── stdarg/stdarg.c
│   │       ├── stddef/offsetof.c
│   │       ├── stdio/
│   │       │   ├── printf.c
│   │       │   ├── puts.c
│   │       │   └── getchar.c
│   │       ├── stdlib/
│   │       │   ├── atoi.c
│   │       │   ├── exit.c
│   │       │   └── malloc.c
│   │       ├── string/
│   │       │   ├── memcpy.c
│   │       │   ├── strlen.c
│   │       │   └── strcmp.c
│   │       ├── syscall/syscall.c
│   │       ├── time/time.c
│   │       └── unistd/
│   │           ├── read.c
│   │           ├── write.c
│   │           └── execve.c
│   └── src/
│       └── sys/
│           ├── arch/
│           │   └── arm64/
│           │       ├── context.S
│           │       ├── mmu.S
│           │       ├── mmu.c
│           │       ├── mmu.h
│           │       ├── ptdefs.h
│           │       └── cpu.h
│           ├── dev/
│           │   └── uart.c
│           ├── fs/
│           │   ├── devfs.c
│           │   ├── devfs.h
│           │   ├── ext2/
│           │   ├── vfs.c
│           │   └── vfs.h
│           ├── include/
│           │   └── trap_frame.h
│           ├── init/
│           │   └── kernel_init.c
│           ├── kern/
│           │   ├── syscall.c
│           │   ├── trap.c
│           │   ├── exec_loader.c
│           │   └── panic.c
│           ├── mm/
│           │   ├── mmu.c
│           │   ├── kmalloc.c
│           │   └── vmm.c
│           ├── process/
│           │   ├── process.c
│           │   ├── process.h
│           │   └── process_image.h
│           ├── scheduler/
│           │   ├── scheduler.c
│           │   ├── scheduler.h
│           │   └── queue.c
│           ├── syscalls/
│           │   ├── syscall.h
│           │   └── syscall_wrapper.h
│           └── thread/
│               ├── thread.c
│               └── thread.h
├── var/
│   └── log/
├── toolchain/
│   ├── Makefile
│   ├── crt0.S
│   └── build.sh
├── build/
│   ├── kernel.elf
│   ├── kernel.img
│   ├── map.txt
│   └── *.o
├── Makefile
└── README.md
