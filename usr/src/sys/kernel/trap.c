// trap.c

#include "trap.h"
#include "regs.h"
#include "thread.h"
#include "scheduler.h"
#include "syscalls.h"
#include "panic.h"

void trap_handle(trapframe_t *tf) {
    cpu_id_t cpu = this_cpu();
    thread_t *curr = current_thread[cpu];

    trap_reason_t reason = arch_decode_trap(tf);

    switch (reason) {
    case TRAP_SYSCALL:
        syscall_dispatch(tf);
        break;
    case TRAP_PAGEFAULT:
        panic("Page fault: addr=%lx elr=%lx\n", arch_get_fault_addr(), tf->elr);
        break;
    case TRAP_UNKNOWN:
    default:
        panic("Unknown trap: ESR=0x%lx\n", tf->esr);
    }

    scheduler_tick(cpu);
    thread_t *next = scheduler_pick_next(cpu);
    if (next != curr) {
        current_thread[cpu] = next;
        context_switch(curr, next);
    }
}

void irq_handle(trapframe_t *tf) {
    cpu_id_t cpu = this_cpu();
    timer_handle_irq();
    scheduler_tick(cpu);

    thread_t *curr = current_thread[cpu];
    thread_t *next = scheduler_pick_next(cpu);

    if (next != curr) {
        current_thread[cpu] = next;
        context_switch(curr, next);
    }
}
