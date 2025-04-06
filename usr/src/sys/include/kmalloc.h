#ifndef KMALLOC_H
#define KMALLOC_H

#include <stddef.h>
#include <stdint.h>
#include "mmu.h"

void kmalloc_init(uintptr_t heap_base, size_t heap_size, PageTable* pt);

void* kmalloc(size_t size);
void* kmalloc_aligned(size_t size, size_t alignment);
void* kcalloc(size_t count, size_t size);
void kfree(void* ptr);

size_t kmalloc_used_bytes(void);
size_t kmalloc_free_bytes(void);

#endif
/*
#define KERNEL_HEAP_BASE 0xFFFF800000000000
#define KERNEL_HEAP_SIZE (16 * 1024 * 1024) // 16MB

extern PageTable kernel_page_table;

kmalloc_init(KERNEL_HEAP_BASE, KERNEL_HEAP_SIZE, &kernel_page_table);
*/