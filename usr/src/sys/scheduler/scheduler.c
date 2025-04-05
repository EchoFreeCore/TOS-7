#include <stdint.h>
#include <stddef.h>
#include "thread.h"
#include "queue.h"
#include "timer.h"
#include "scheduler.h"

#define MAX_CPUS 4
#define PRIORITY_LEVELS 32
#define NUM_CLASSES 3 // SCHED_TS, SCHED_RT, SCHED_SYS

// === Del 1: Schemaläggningsklass-API ===

typedef struct scheduler_class {
    const char* name;
    void (*enqueue)(cpu_id_t, thread_t*);
    thread_t* (*pick_next)(cpu_id_t);
    void (*tick)(cpu_id_t);
    bool (*should_preempt)(cpu_id_t, thread_t*, thread_t*);
} scheduler_class_t;

// === Del 2: Per-CPU runqueues och current thread ===

static struct queue runqueues[MAX_CPUS][NUM_CLASSES][PRIORITY_LEVELS];
static thread_t* current_thread[MAX_CPUS];
extern thread_t* idle_thread[MAX_CPUS];

// === Del 3: Sleepqueue-struktur per CPU ===

typedef struct sleep_entry {
    thread_t* thread;
    uint64_t wakeup_time;
    struct sleep_entry* next;
} sleep_entry_t;

static sleep_entry_t* sleepqueue[MAX_CPUS];

// === Del 4: Time-sharing (TS) schemaklass ===

static void ts_enqueue(cpu_id_t cpu, thread_t* t) {
    int prio = t->priority;
    queue_put(&runqueues[cpu][SCHED_TS][prio], &t->qnode);
}

static thread_t* ts_pick_next(cpu_id_t cpu) {
    for (int prio = PRIORITY_LEVELS - 1; prio >= 0; --prio) {
        if (!queue_empty(&runqueues[cpu][SCHED_TS][prio])) {
            return (thread_t*)queue_get(&runqueues[cpu][SCHED_TS][prio]);
        }
    }
    return NULL;
}

static void ts_tick(cpu_id_t cpu) {
    thread_t* curr = current_thread[cpu];
    if (!curr || curr->sched_class != SCHED_TS) return;

    if (--curr->quantum <= 0) {
        curr->quantum = curr->base_quantum;
        scheduler_enqueue(cpu, curr);
        current_thread[cpu] = scheduler_pick_next(cpu);
        context_switch(curr, current_thread[cpu]);
    }
}

static bool ts_preempt(cpu_id_t cpu, thread_t* curr, thread_t* next) {
    return next->priority > curr->priority;
}

// === Del 5: Real-Time (RT) och System (SYS) schemaklasser ===

static void rt_enqueue(cpu_id_t cpu, thread_t* t) {
    queue_put(&runqueues[cpu][SCHED_RT][t->priority], &t->qnode);
}

static thread_t* rt_pick_next(cpu_id_t cpu) {
    for (int prio = PRIORITY_LEVELS - 1; prio >= 0; --prio) {
        if (!queue_empty(&runqueues[cpu][SCHED_RT][prio])) {
            return (thread_t*)queue_get(&runqueues[cpu][SCHED_RT][prio]);
        }
    }
    return NULL;
}

static bool rt_preempt(cpu_id_t cpu, thread_t* curr, thread_t* next) {
    return true; // RT trådar preempterar alltid
}

// SYS delar logik med TS
#define sys_enqueue ts_enqueue
#define sys_pick_next ts_pick_next
#define sys_tick ts_tick
#define sys_preempt ts_preempt

// === Del 6: Schemaklass-tabell ===

scheduler_class_t sched_classes[NUM_CLASSES] = {
    [SCHED_TS] = { "TS", ts_enqueue, ts_pick_next, ts_tick, ts_preempt },
    [SCHED_RT] = { "RT", rt_enqueue, rt_pick_next, NULL, rt_preempt },
    [SCHED_SYS] = { "SYS", sys_enqueue, sys_pick_next, sys_tick, sys_preempt }
};

// === Del 7: Sleepqueue-hantering (väckning) ===

void scheduler_sleep(cpu_id_t cpu, thread_t* t, uint64_t wake_time) {
    sleep_entry_t* entry = alloc_sleep_entry(); // användardefinierad
    entry->thread = t;
    entry->wakeup_time = wake_time;
    entry->next = NULL;

    sleep_entry_t** head = &sleepqueue[cpu];
    while (*head && (*head)->wakeup_time <= wake_time)
        head = &(*head)->next;

    entry->next = *head;
    *head = entry;

    t->state = PROC_SLEEPING;
}

void scheduler_timer_tick(cpu_id_t cpu) {
    uint64_t now = timer_now();
    while (sleepqueue[cpu] && sleepqueue[cpu]->wakeup_time <= now) {
        sleep_entry_t* entry = sleepqueue[cpu];
        sleepqueue[cpu] = entry->next;

        thread_t* t = entry->thread;
        t->state = PROC_RUNNING;
        scheduler_enqueue(cpu, t);
        free_sleep_entry(entry); // användardefinierad
    }
}

// === Del 8: Core-scheduling – enqueue, pick ===

void scheduler_enqueue(cpu_id_t cpu, thread_t* t) {
    t->quantum = t->base_quantum;
    sched_classes[t->sched_class].enqueue(cpu, t);
}

thread_t* scheduler_pick_next(cpu_id_t cpu) {
    for (int cls = SCHED_RT; cls <= SCHED_SYS; ++cls) {
        thread_t* t = sched_classes[cls].pick_next(cpu);
        if (t) return t;
    }
    return idle_thread[cpu];
}

// === Del 9: scheduler_tick – timer, klass-tick, preemption ===
/*
void scheduler_tick(cpu_id_t cpu) {
    thread_t* curr = current_thread[cpu];
    thread_t* next = scheduler_pick_next(cpu);
    if (curr != next) {
        context_switch(curr, next);
    }
}
    */

void scheduler_tick(cpu_id_t cpu) {
    scheduler_timer_tick(cpu);

    thread_t* curr = current_thread[cpu];
    if (curr && sched_classes[curr->sched_class].tick)
        sched_classes[curr->sched_class].tick(cpu);

    thread_t* next = scheduler_pick_next(cpu);
    if (next != curr &&
        sched_classes[next->sched_class].should_preempt(cpu, curr, next)) {
        current_thread[cpu] = next;
        context_switch(curr, next);
    }
}