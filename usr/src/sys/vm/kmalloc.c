#include "kmalloc.h"
#include "pmm.h"
#include "mmu.h"
#include "panic.h"
#include <string.h>
#include <stddef.h>
#include <stdint.h>

#define KMAGIC 0xCAFEBABE
#define NODE_PAGE_SIZE 0x1000

typedef struct KmallocNode {
    struct KmallocNode* next;
    struct KmallocNode* prev;
    uintptr_t start;
    size_t size;
    int is_free;
    uint32_t magic;
} KmallocNode;

static KmallocNode* free_head = NULL;
static KmallocNode* alloc_head = NULL;
static uintptr_t heap_break;
static uintptr_t heap_end;

static KmallocNode* node_arena = NULL;
static size_t node_arena_cap = 0;
static size_t node_arena_used = 0;

static PageTable* kmalloc_page_table = NULL;

static inline uintptr_t align_up(uintptr_t val, uintptr_t align) {
    return (val + align - 1) & ~(align - 1);
}

static void expand_node_arena(void) {
    uintptr_t phys = pmm_alloc_page();
    if (!phys) panic("kmalloc: out of node memory");

    uintptr_t virt = phys; // identitetsmappning
    mmu_map(kmalloc_page_table->root, virt, phys, PTE_VALID | PTE_WRITE);

    if (!node_arena) {
        node_arena = (KmallocNode*)virt;
    }

    node_arena_cap += NODE_PAGE_SIZE / sizeof(KmallocNode);
}

static KmallocNode* alloc_node(void) {
    if (node_arena_used >= node_arena_cap)
        expand_node_arena();

    KmallocNode* n = &node_arena[node_arena_used++];
    memset(n, 0, sizeof(KmallocNode));
    n->magic = KMAGIC;
    return n;
}

static void insert_after(KmallocNode* anchor, KmallocNode* node) {
    node->next = anchor->next;
    node->prev = anchor;
    if (anchor->next) anchor->next->prev = node;
    anchor->next = node;
}

static void remove_node(KmallocNode* node) {
    if (node->prev) node->prev->next = node->next;
    if (node->next) node->next->prev = node->prev;
}

static void try_merge(KmallocNode* node) {
    if (node->next && node->next->is_free &&
        node->start + node->size == node->next->start) {
        node->size += node->next->size;
        remove_node(node->next);
    }

    if (node->prev && node->prev->is_free &&
        node->prev->start + node->prev->size == node->start) {
        node->prev->size += node->size;
        remove_node(node);
    }
}

void kmalloc_init(uintptr_t heap_base, size_t heap_size, PageTable* pt) {
    heap_break = heap_base;
    heap_end = heap_base + heap_size;
    kmalloc_page_table = pt;

    expand_node_arena();

    KmallocNode* n = alloc_node();
    n->start = heap_base;
    n->size = heap_size;
    n->is_free = 1;

    free_head = n;
    n->prev = NULL;
    n->next = NULL;

    alloc_head = NULL;
}

static void* internal_alloc(size_t size, size_t align) {
    KmallocNode* current = free_head;
    while (current) {
        if (!current->is_free) {
            current = current->next;
            continue;
        }

        uintptr_t aligned_start = align_up(current->start, align);
        size_t padding = aligned_start - current->start;

        if (current->size >= padding + size) {
            if (padding > 0) {
                KmallocNode* pad = alloc_node();
                pad->start = current->start;
                pad->size = padding;
                pad->is_free = 1;

                pad->next = current;
                pad->prev = current->prev;
                if (current->prev) current->prev->next = pad;
                current->prev = pad;

                current->start += padding;
                current->size -= padding;
            }

            if (current->size > size) {
                KmallocNode* remain = alloc_node();
                remain->start = current->start + size;
                remain->size = current->size - size;
                remain->is_free = 1;

                remain->next = current->next;
                remain->prev = current;
                if (current->next) current->next->prev = remain;
                current->next = remain;

                current->size = size;
            }

            current->is_free = 0;

            // Map backing pages via mmu if not mapped
            for (uintptr_t addr = current->start; addr < current->start + current->size; addr += PAGE_SIZE) {
                if (!mmu_is_valid(virt_to_phys(kmalloc_page_table->root, addr))) {
                    uintptr_t phys = pmm_alloc_page();
                    if (!phys) panic("kmalloc: out of physical memory");
                    mmu_map(kmalloc_page_table->root, addr, phys, PTE_VALID | PTE_WRITE);
                }
            }

            return (void*)current->start;
        }

        current = current->next;
    }

    panic("kmalloc: out of memory");
    return NULL;
}

void* kmalloc(size_t size) {
    if (!size) return NULL;
    return internal_alloc(size, sizeof(void*));
}

void* kmalloc_aligned(size_t size, size_t alignment) {
    if (!size || alignment == 0 || (alignment & (alignment - 1)) != 0)
        panic("kmalloc_aligned: invalid alignment");
    return internal_alloc(size, alignment);
}

void* kcalloc(size_t count, size_t size) {
    if (!count || !size || count > SIZE_MAX / size)
        panic("kcalloc: invalid args");

    void* ptr = kmalloc(count * size);
    if (ptr) memset(ptr, 0, count * size);
    return ptr;
}

void kfree(void* ptr) {
    if (!ptr) return;

    uintptr_t addr = (uintptr_t)ptr;
    KmallocNode* n = free_head;
    while (n) {
        if (n->start == addr) break;
        n = n->next;
    }

    if (!n || n->magic != KMAGIC || n->is_free)
        panic("kfree: invalid or double free");

    n->is_free = 1;
    try_merge(n);
}

size_t kmalloc_used_bytes(void) {
    size_t total = 0;
    KmallocNode* n = free_head;
    while (n) {
        if (!n->is_free) total += n->size;
        n = n->next;
    }
    return total;
}

size_t kmalloc_free_bytes(void) {
    size_t total = 0;
    KmallocNode* n = free_head;
    while (n) {
        if (n->is_free) total += n->size;
        n = n->next;
    }
    return total;
}
