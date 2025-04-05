#include "trap_frame.h"
#include "thread.h"
#include "scheduler.h"
#include "mmu.h"
#include "string.h"

// extern ASM-etikett för att återuppta i trap return
extern void context_restore_and_eret(trap_frame_t * frame);

// context switch med trap frame
void context_switch(thread_t* from, thread_t* to) {
    int cpu = this_cpu();
    current_thread[cpu] = to;

    // Byt page table
    write_ttbr0_el1((uint64_t)to->page_table);

    // Ställ in SP_EL0 (användarstack)
    asm volatile("msr sp_el0, %0" :: "r"(to->trap_frame->sp_el0));

    // Gå tillbaka till trap return
    context_restore_and_eret(to->trap_frame);
    __builtin_unreachable();
}
