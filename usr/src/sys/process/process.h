
#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include <stdbool.h>
#include "vfs.h"

#define MAX_FD 128
#define MAX_PROC_NAME 32

typedef int pid_t;

// === Processstatus ===
typedef enum {
    PROC_RUNNING,
    PROC_SLEEPING,
    PROC_WAITING,
    PROC_ZOMBIE,
    PROC_EXITED
} process_state_t;

// === Processstruktur ===

typedef struct process {
    pid_t pid;
    char name[MAX_PROC_NAME];

    process_state_t state;

    // Filbeskrivartabell (POSIX fd)
    file_t *fd_table[MAX_FD];

    // Användarstack eller adressrymd (valfritt)
    void *heap_start;
    void *heap_end;

    // Signalhantering (valfritt för POSIX signaler)
    // sigset_t signal_mask;
    // void (*signal_handlers[32])(int);

    // Exit-status
    int exit_code;
    bool exited;

    // Föräldrarelation (om du vill implementera waitpid())
    pid_t parent_pid;

    // Kan utökas med e.g. cwd, env, cred
} process_t;

#endif // PROCESS_H


/*
typedef struct process {
    ...
        pagetable_t page_table;

    uintptr_t mmap_base;
    uintptr_t mmap_brk;   // högsta allokerade mmap-adress
    ...
} process_t;

process_create() eller exec_load_elf()
proc->mmap_base = 0x100000000; // typiskt mmap-område
proc->mmap_brk = proc->mmap_base;


*/