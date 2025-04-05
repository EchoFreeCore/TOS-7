#include "process_context.h"
#include "platform.h" // innehåller this_cpu()

process_t *current_process[MAX_CPUS] = {0};

// Läs aktuell CPU (plattformsspecifik funktion)
extern cpu_id_t this_cpu(void);

process_t *get_current_process(void) {
    return current_process[this_cpu()];
}

void set_current_process(cpu_id_t cpu, process_t *proc) {
    current_process[cpu] = proc;
}
