.section .text
.global _start
.type _start, %function

_start:
    // Call main()
    bl main

    // Return value from main is in x0
    // Call _exit(ret)
    bl _exit

    // If _exit ever returns, just loop
1:
    wfe
    b 1b