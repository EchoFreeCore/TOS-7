#ifndef SPINLOCK_H
#define SPINLOCK_H

#include <stdint.h>
#include <stdbool.h>

typedef volatile int spinlock_t;

static inline void spinlock_init(spinlock_t *lock) {
    *lock = 0;
}

static inline void spin_lock(spinlock_t *lock) {
    while (__sync_lock_test_and_set(lock, 1)) {
        while (*lock);
    }
}

static inline void spin_unlock(spinlock_t *lock) {
    __sync_lock_release(lock);
}

#endif
