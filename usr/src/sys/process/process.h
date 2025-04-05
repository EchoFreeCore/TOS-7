
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