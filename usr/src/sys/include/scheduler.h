#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include "thread.h"

// === Del 1: Schemaklass-ID:n ===
// Identifierar olika schemaläggningsklasser, inspirerat av Solaris.
typedef enum {
    SCHED_TS = 0,   // Time-sharing
    SCHED_RT = 1,   // Real-time (fixed priority)
    SCHED_SYS = 2   // System (idle/system threads)
} sched_class_id_t;

// === Del 2: CPU-ID typdefinition ===
typedef int cpu_id_t;

// === Del 3: Publika funktioner för scheduling ===

// Enqueue en tråd i rätt runqueue baserat på schemaklass
void scheduler_enqueue(cpu_id_t cpu, thread_t* t);

// Väljer nästa tråd att köra på en CPU
thread_t* scheduler_pick_next(cpu_id_t cpu);

// Kallas varje systemtick – sköter:
// - Väckning av sovande trådar
// - Timeslice nedräkning
// - Eventuell preemption
void scheduler_tick(cpu_id_t cpu);

// Hanterar väckning av sovande trådar (timerdriven)
void scheduler_timer_tick(cpu_id_t cpu);

// Schemalägger en tråd att sova till en viss tidpunkt
void scheduler_sleep(cpu_id_t cpu, thread_t* t, uint64_t wake_time);

// === Del 4: Schemaklass-tabell ===
// Global tabell som används internt för att mappa schemaklass till funktioner
extern struct scheduler_class sched_classes[];

// === Del 5: Externa variabler ===
extern struct thread* current_thread[MAX_CPUS];
extern struct thread* idle_thread[MAX_CPUS];

// === Del 6: Sleepqueue-hantering ===
// Dessa måste implementeras av användaren eller elsewhere i kärnan

// Allokerar en sleepqueue-entry (heap eller pool)
struct sleep_entry* alloc_sleep_entry(void);

// Frigör en sleepqueue-entry
void free_sleep_entry(struct sleep_entry* entry);

#endif // SCHEDULER_H