#include "spinlock.h"
#include "cpuwrap.h"

void spinlock_acquire(spinlock_t *lock) {
    while (atomic_testset(&lock->lock)) {
        cpu_wait();
    }
    memory_barrier();  // ordering efter lock
}

void spinlock_release(spinlock_t *lock) {
    memory_barrier();  // ordering före unlock
    lock->lock = 0;
}

int spinlock_try(spinlock_t *lock) {
    if (!atomic_testset(&lock->lock)) {
        memory_barrier();
        return 1;
    }
    return 0;
}

// IRQ-säker spinlock
void irqspinlock_acquire(irqspinlock_t *lock) {
    uint32_t flags = irq_is_enabled();
    if (flags)
        irq_disable();

    while (atomic_testset(&lock->lock)) {
        cpu_wait();
    }

    memory_barrier();
    lock->irq_flags = flags;
}

void irqspinlock_release(irqspinlock_t *lock) {
    memory_barrier();
    lock->lock = 0;

    if (lock->irq_flags)
        irq_enable();
}
