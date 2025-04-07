.section .text
.global _start
.type _start, %function

_start:
    // x0 = argc (first 8 bytes at sp)
    ldr x0, [sp]

    // x1 = argv = sp + 8
    add x1, sp, #8

    // x2 = envp = after NULL terminator in argv
    // find envp by scanning argv until NULL
    mov x3, x1        // x3 = ptr = argv
find_null:
    ldr x4, [x3], #8  // load argv[i], advance
    cbnz x4, find_null
    mov x2, x3        // x2 = envp (right after NULL)

    // store environ = envp
    adrp x4, environ
    add x4, x4, :lo12:environ
    str x2, [x4]

    // call main(argc, argv, envp)
    bl main

    // main's return value is in x0
    bl _exit

1:
    wfe
    b 1b
