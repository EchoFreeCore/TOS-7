#include <stdlib.h>   // malloc, free
#include <string.h>   // memset
#include "thread.h"
#include "scheduler.h"

#define STACK_SIZE 8192 // 8 KB stack, kan justeras

static tid_t next_tid = 1;

// === Del 1: Allokera och initiera tråd ===

thread_t* thread_alloc(void) {
    thread_t* t = malloc(sizeof(thread_t));
    if (!t) return NULL;

    memset(t, 0, sizeof(thread_t));

    t->stack_ptr = malloc(STACK_SIZE);
    if (!t->stack_ptr) {
        free(t);
        return NULL;
    }

    return t;
}

// === Del 2: Skapa en ny tråd (användartråd) ===

int thread_create(pid_t pid, void (*entry)(void*), void* arg) {
    thread_t* t = thread_alloc();
    if (!t) return -1;

    t->tid = next_tid++;
    t->pid = pid;
    t->state = PROC_WAITING;
    t->entry_point = entry;
    t->arg = arg;

    t->assigned_cpu = 0;             // Initial CPU – kan bli SMP-aware senare
    t->sched_class = SCHED_TS;       // Standard time-sharing
    t->priority = 10;                // Medium prioritet
    t->base_quantum = 5;             // T.ex. 5 ticks per timeslice
    t->quantum = t->base_quantum;
    t->detached = false;

    arch_init_thread_context(t, entry, arg);
    register_thread(t);
    scheduler_enqueue(t->assigned_cpu, t);

    return t->tid;
}

// === Del 3: Förstör en tråd ===

void thread_destroy(thread_t* t) {
    if (!t) return;

    if (t->stack_ptr)
        free(t->stack_ptr);

    unregister_thread(t->tid);
    free(t);
}

// === Del 4: Tråd-yield (frivillig kontextswitch) ===

int thread_yield(void) {
    cpu_id_t cpu = this_cpu(); // plattformsfunktion som returnerar current CPU
    thread_t* curr = current_thread[cpu];

    scheduler_enqueue(cpu, curr);
    thread_t* next = scheduler_pick_next(cpu);
    current_thread[cpu] = next;
    context_switch(curr, next);

    return 0;
}

// === Del 5: Tråd-sömn i millisekunder ===

int thread_sleep(uint64_t ms) {
    cpu_id_t cpu = this_cpu();
    thread_t* curr = current_thread[cpu];

    uint64_t now = timer_now();
    uint64_t wake_time = now + ms;

    scheduler_sleep(cpu, curr, wake_time);
    thread_t* next = scheduler_pick_next(cpu);
    current_thread[cpu] = next;
    context_switch(curr, next);

    return 0;
}

// === Del 6: Blockera och avblockera trådar (för sync etc) ===

int thread_block(proc_state_t reason) {
    cpu_id_t cpu = this_cpu();
    thread_t* curr = current_thread[cpu];

    curr->state = reason;
    thread_t* next = scheduler_pick_next(cpu);
    current_thread[cpu] = next;
    context_switch(curr, next);

    return 0;
}

int thread_unblock(thread_t* t) {
    if (!t) return -1;
    t->state = PROC_RUNNING;
    scheduler_enqueue(t->assigned_cpu, t);
    return 0;
}

// === Del 7: Join och detach (kan förbättras senare) ===

int thread_join(tid_t tid, void** retval) {
    thread_t* target = thread_lookup(tid);
    if (!target) return -1;

    while (target->state != PROC_ZOMBIE) {
        thread_block(PROC_WAITING);
    }

    if (retval)
        *retval = target->retval;

    thread_destroy(target);
    return 0;
}

int thread_detach(tid_t tid) {
    thread_t* t = thread_lookup(tid);
    if (!t) return -1;

    t->detached = true;
    return 0;
}

// === Del 8: Avsluta tråd ===

void thread_exit(void* retval) {
    cpu_id_t cpu = this_cpu();
    thread_t* curr = current_thread[cpu];

    curr->retval = retval;
    curr->state = PROC_ZOMBIE;

    if (curr->detached) {
        thread_destroy(curr);
    }

    thread_t* next = scheduler_pick_next(cpu);
    current_thread[cpu] = next;
    context_switch(curr, next);
}

// thread.h

#ifndef THREAD_H
#define THREAD_H

#include <stdint.h>
#include <stdbool.h>
#include "process.h"

// === Del 1: Tråd-ID och CPU-typer ===

typedef int tid_t;
typedef int pid_t;
typedef int cpu_id_t;

// === Del 2: Trådtillstånd (inspirerat av klassisk UNIX/Solaris) ===

typedef enum {
    PROC_NEW = 0,
    PROC_RUNNING,
    PROC_WAITING,
    PROC_SLEEPING,
    PROC_ZOMBIE
} proc_state_t;

// === Del 3: Schemaklass-identifiering ===

typedef enum {
    SCHED_TS = 0,
    SCHED_RT = 1,
    SCHED_SYS = 2
} sched_class_id_t;

// === Del 4: Trådstruktur ===
// Denna måste matcha queue- och schemaläggningssystemet

typedef struct thread {
    tid_t tid;
    pid_t pid;

    proc_state_t state;
    sched_class_id_t sched_class;

    int priority;         // Statisk eller dynamisk prioritet
    int base_quantum;     // Ursprungligt kvantum
    int quantum;          // Återstående tid i ticks

    cpu_id_t assigned_cpu;

    void (*entry_point)(void*);
    void* arg;
    void* retval;
    bool detached;

    void* stack_ptr;

    struct queue_node qnode; // För intrusive queue
    // + arkitekturberoende fält (register context etc.)
} thread_t;

// === Del 5: Tråd-API – POSIX-liknande interface ===

int thread_yield(void);
int thread_sleep(uint64_t ms);
int thread_block(proc_state_t reason);
int thread_unblock(thread_t* t);
int thread_join(tid_t tid, void** retval);
int thread_detach(tid_t tid);
void thread_exit(void* retval);

// === Del 6: Trådskapa/förstör ===

int thread_create(pid_t pid, void (*entry)(void*), void* arg);
void thread_destroy(thread_t* t);

// === Del 7: Arkitekturberoende bootstrap ===

void arch_init_thread_context(thread_t* t, void (*entry)(void*), void* arg);

// === Del 8: Global trådtabell (extern hantering) ===

void register_thread(thread_t* t);
void unregister_thread(tid_t tid);
thread_t* thread_lookup(tid_t tid);

#endif // THREAD_H