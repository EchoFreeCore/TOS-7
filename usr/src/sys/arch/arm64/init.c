void arm64_init(void) {
    cpu_timer_init(1000000);    // init timer-hårdvara (CNTV/CNTP)
    irq_enable();               // slår på global interruptmask
    main()
}