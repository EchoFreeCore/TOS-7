#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <stdint.h>

// Vanlig spinlock
typedef struct {
    volatile uint32_t lock;
} spinlock_t;

#define SPINLOCK_INIT {0}

void spinlock_acquire(spinlock_t *lock);
void spinlock_release(spinlock_t *lock);
int  spinlock_try(spinlock_t *lock);

// IRQ-säker spinlock
typedef struct {
    volatile uint32_t lock;
    uint32_t irq_flags;
} irqspinlock_t;

#define IRQSPINLOCK_INIT {0, 0}

void irqspinlock_acquire(irqspinlock_t *lock);
void irqspinlock_release(irqspinlock_t *lock);

#endif
