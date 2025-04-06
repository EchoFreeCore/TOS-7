#include "pmm.h"
#include "panic.h"

#define MAX_PAGES 65536 // 256MB / 4KB
#define PAGE_SIZE 4096

static uintptr_t free_list[MAX_PAGES];
static size_t free_list_top = 0;
static uintptr_t base_addr;

void pmm_init(uintptr_t mem_start, size_t mem_size) {
    base_addr = mem_start;
    size_t num_pages = mem_size / PAGE_SIZE;

    if (num_pages > MAX_PAGES)
        panic("PMM: too many pages");

    for (size_t i = 0; i < num_pages; ++i) {
        free_list[free_list_top++] = mem_start + (i * PAGE_SIZE);
    }
}

uintptr_t pmm_alloc_page(void) {
    if (free_list_top == 0)
        return 0;

    return free_list[--free_list_top];
}

void pmm_free_page(uintptr_t phys_addr) {
    if (free_list_top >= MAX_PAGES)
        panic("PMM: free list overflow");

    free_list[free_list_top++] = phys_addr;
}
/*
t.ex. 0x100000 till 0x4000000 = 63MB
pmm_init(0x100000, 0x3F00000);
*/