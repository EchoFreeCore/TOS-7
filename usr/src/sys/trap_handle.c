#include "syscalls.h"
#include "trap.h"

int trap_handle(trap_frame_t* tf) {
    uint64_t syscall_nr = tf->regs[8]; // x8 är syscall #
    uint64_t args[6] = {
        tf->regs[0], tf->regs[1], tf->regs[2],
        tf->regs[3], tf->regs[4], tf->regs[5]
    };

    tf->regs[0] = syscall_dispatch(syscall_nr, args);
    return 0;
}