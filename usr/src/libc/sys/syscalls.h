ifndef _SYSCALL_WRAPPER_H
#define _SYSCALL_WRAPPER_H

#include <stdint.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include "syscall.h" // din inline asm syscall()

#define SYS0(NUM, NAME, RET, ARGS, CALL) \
RET NAME ARGS { return (RET) syscall(NUM, 0, 0, 0, 0, 0, 0); }

#define SYS1(NUM, NAME, RET, ARGS, CALL) \
RET NAME ARGS { return (RET) syscall(NUM, (uintptr_t)CALL); }

#define SYS2(NUM, NAME, RET, ARGS, CALL...) \
RET NAME ARGS { return (RET) syscall(NUM, (uintptr_t)CALL, 0, 0, 0, 0); }

#define SYS3(NUM, NAME, RET, ARGS, CALL...) \
RET NAME ARGS { return (RET) syscall(NUM, (uintptr_t)CALL, 0, 0, 0); }

#define SYS4(NUM, NAME, RET, ARGS, CALL...) \
RET NAME ARGS { return (RET) syscall(NUM, (uintptr_t)CALL, 0, 0); }

#define SYS5(NUM, NAME, RET, ARGS, CALL...) \
RET NAME ARGS { return (RET) syscall(NUM, (uintptr_t)CALL, 0); }

#define SYS6(NUM, NAME, RET, ARGS, CALL...) \
RET NAME ARGS { return (RET) syscall(NUM, (uintptr_t)CALL); }

// Genom att redefiniera SYS så kan vi göra:
#define SYS(NUM, ARGC, NAME, SIGNATURE) \
    SYS##ARGC(NUM, NAME, long, SIGNATURE, SIGNATURE)

#include "syscalls.tbl"

#undef SYS0
#undef SYS1
#undef SYS2
#undef SYS3
#undef SYS4
#undef SYS5
#undef SYS6
#undef SYS

#endif /* _SYSCALL_WRAPPER_H */

sys / syscall.h — syscall - nummer header
#ifndef _SYS_SYSCALL_H
#define _SYS_SYSCALL_H

// Auto-generated syscall numbers
#define DECL_SYSCALL(NUM, ARGS, NAME, SIGN)  SYS_##NAME NUM

// Expand
#define SYS(NUM, ARGS, NAME, SIGN)  DECL_SYSCALL(NUM, ARGS, NAME, SIGN)
#include "syscalls.tbl"
#undef SYS
#undef DECL_SYSCALL

#endif /* _SYS_SYSCALL_H */