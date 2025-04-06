#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <stdint.h>
#include <stddef.h>

// Generisk syscall wrapper för AArch64 FreeBSD.
// FreeBSD använder samma syscall ABI som Linux på ARM64, men syscall-nummer skiljer sig.
static inline long syscall(int num,
    uintptr_t a1, uintptr_t a2, uintptr_t a3,
    uintptr_t a4, uintptr_t a5, uintptr_t a6)
{
    register uintptr_t x8 asm("x8") = num;
    register uintptr_t x0 asm("x0") = a1;
    register uintptr_t x1 asm("x1") = a2;
    register uintptr_t x2 asm("x2") = a3;
    register uintptr_t x3 asm("x3") = a4;
    register uintptr_t x4 asm("x4") = a5;
    register uintptr_t x5 asm("x5") = a6;

    asm volatile("svc #0"
        : "+r"(x0)
        : "r"(x1), "r"(x2), "r"(x3),
          "r"(x4), "r"(x5), "r"(x8)
        : "memory");

    return x0;
}

#endif // _SYSCALL_H