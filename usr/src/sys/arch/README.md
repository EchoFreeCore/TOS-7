start.S          | Kernel entry point (_start), sets SP, clears BSS
trap.S           | Exception/trap vector table (SVC, IRQ, FIQ)
boot.c           | First-stage kernel init, MMU off, calls kernel_main()
trap.c           | Trap dispatcher, syscall vectoring, fault decoding
context_switch.S | Assembly context switch for threads/processes
thread_start_stub.S | Bootstrap for new threads, calls entry function