#ifndef PROCESS_CONTEXT_H
#define PROCESS_CONTEXT_H

#include "process.h"

#define MAX_CPUS 4  // Justera efter behov

// Per-CPU aktiv process
extern process_t *current_process[MAX_CPUS];

// Returnerar process kopplad till nuvarande CPU
process_t *get_current_process(void);

// Sätt aktuell process på en CPU (t.ex. från scheduler)
void set_current_process(cpu_id_t cpu, process_t *proc);

#endif
