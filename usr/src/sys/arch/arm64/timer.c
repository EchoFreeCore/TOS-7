#include <sys/arch/arm64/cpu.h>
#include <kernel/scheduler.h>
#include <kernel/irq.h>

static volatile uint64_t ticks = 0;

static void timer_handler(int irq, void *arg) {
    ticks++;
    scheduler_tick();  // kopplar in din schemaläggare här
    cpu_timer_set_next_event(10000); // rearm timer om den inte är periodisk
}

void timer_init(void) {
    irq_register(IRQ_TIMER, timer_handler, NULL);
    cpu_timer_init(1000000); // ex: 1 MHz timer, ger 10 ms slice
}

uint64_t timer_ticks(void) {
    return ticks;
}