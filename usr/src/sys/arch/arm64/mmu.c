#include "cpuwrap.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdatomic.h>

// This file provides a high-level MMU abstraction for kernel use
// Built on top of cpuwrap.h (which handles arch-specific low-level ops)

#define PAGE_SIZE       4096
#define PAGE_ALIGN(x)   (((x) + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1))
#define PAGE_MASK       (~(PAGE_SIZE - 1))
#define PAGE_SHIFT      12
#define PT_LEVELS       3
#define PTE_PER_TABLE   512
#define VA_BITS         48
#define LEVEL_SHIFT(lvl) (PAGE_SHIFT + ((PT_LEVELS - 1 - (lvl)) * 9))
#define PTE_INDEX(va, lvl) (((va) >> LEVEL_SHIFT(lvl)) & 0x1FF)
#define PTE_VALID       (1ULL << 0)
#define PTE_TABLE       (1ULL << 1)
#define PTE_WRITE       (1ULL << 10)  // assumed write-bit for clone
#define PTE_USER        (1ULL << 6)

// -- Simplified internal representation --
typedef struct PageTable {
    uint64_t *root;       // top-level table (L1)
    uint64_t asid;        // address space ID
    bool valid;           // is it active?
} PageTable;

typedef struct RefPage {
    atomic_int refcount;
    uint8_t data[PAGE_SIZE];
} RefPage;

// -- Static kernel page table --
static PageTable kernel_pt;

static void page_free(void *page) {
    free(page);
}

static RefPage *alloc_ref_page(void) {
    RefPage *pg = aligned_alloc(PAGE_SIZE, sizeof(RefPage));
    if (!pg)
        return NULL;
    memset(pg, 0, sizeof(RefPage));
    atomic_store(&pg->refcount, 1);
    return pg;
}

static void incref(void *page) {
    RefPage *pg = (RefPage *)page;
    atomic_fetch_add(&pg->refcount, 1);
}

static void decref(void *page) {
    RefPage *pg = (RefPage *)page;
    if (atomic_fetch_sub(&pg->refcount, 1) == 1)
        page_free(pg);
}

uintptr_t phys_to_virt(uintptr_t pa) {
    return pa; // Identity mapping for now
}

// Minimal mmuwalk for 3-level page tables
uint64_t *mmuwalk(uint64_t *root, uintptr_t va, int target_level, bool alloc) {
    uint64_t *table = root;
    for (int level = 0; level < target_level; level++) {
        size_t idx = PTE_INDEX(va, level);
        uint64_t pte = table[idx];
        if ((pte & PTE_VALID) && (pte & PTE_TABLE)) {
            table = (uint64_t *)phys_to_virt(pte & PAGE_MASK);
        } else if (alloc) {
            uint64_t *new_table = alloc_page_table();
            if (!new_table)
                return NULL;
            table[idx] = ((uintptr_t)new_table & PAGE_MASK) | PTE_VALID | PTE_TABLE;
            table = new_table;
        } else {
            return NULL;
        }
    }
    return &table[PTE_INDEX(va, target_level)];
}

// Iterate all valid PTE mappings in a pagetable
void mmu_walk_all(uint64_t *root, int level, uintptr_t base_va,
                  void (*callback)(uintptr_t va, uint64_t pte, int level, void *arg), void *arg) {
    for (int i = 0; i < PTE_PER_TABLE; i++) {
        uint64_t pte = root[i];
        uintptr_t va = base_va | ((uintptr_t)i << LEVEL_SHIFT(level));

        if (!(pte & PTE_VALID))
            continue;

        if ((level < 2) && (pte & PTE_TABLE)) {
            uint64_t *child = (uint64_t *)phys_to_virt(pte & PAGE_MASK);
            mmu_walk_all(child, level + 1, va, callback, arg);
        } else {
            callback(va, pte, level, arg);
        }
    }
}

// Initialize kernel MMU and paging system
void mmu_init(void) {
    kernel_pt.root = alloc_page_table();
    kernel_pt.asid = 0;
    kernel_pt.valid = true;

    mmu_disable();
    mmu_set_translation_base((uintptr_t)kernel_pt.root);
    mmu_enable();
}

void mmu_map(uint64_t *l1_table, uintptr_t va, uintptr_t pa, uint64_t attr) {
    uint64_t *pte = mmuwalk(l1_table, va, 2, true);
    if (!pte)
        return;
    *pte = (pa & PAGE_MASK) | attr | PTE_VALID;
}

void mmu_unmap(uint64_t *l1_table, uintptr_t va) {
    uint64_t *pte = mmuwalk(l1_table, va, 2, false);
    if (pte)
        *pte = 0;
    mmu_flush_tlb_local();
}

uintptr_t virt_to_phys(uint64_t *l1_table, uintptr_t va) {
    uint64_t *pte = mmuwalk(l1_table, va, 2, false);
    if (!pte || !(*pte & PTE_VALID))
        return 0;
    return (*pte & PAGE_MASK) | (va & ~PAGE_MASK);
}

void mmu_map_identity(uintptr_t phys, uintptr_t size, uint64_t attr_flags) {
    uintptr_t pa = phys & PAGE_MASK;
    uintptr_t end = PAGE_ALIGN(phys + size);

    for (; pa < end; pa += PAGE_SIZE) {
        mmu_map(kernel_pt.root, pa, pa, attr_flags);
    }
    mmu_flush_tlb_local();
}

void mmu_assign_asid(PageTable *pt, uint64_t asid) {
    pt->asid = asid;
}

void *mmu_map_region(uintptr_t phys, size_t size, uint64_t flags) {
    static uintptr_t vbase = 0xFFFF000000000000;
    uintptr_t va = vbase;
    uintptr_t aligned_size = PAGE_ALIGN(size);
    for (uintptr_t offset = 0; offset < aligned_size; offset += PAGE_SIZE)
        mmu_map(kernel_pt.root, va + offset, phys + offset, flags);
    mmu_flush_tlb_local();
    vbase += aligned_size;
    return (void *)va;
}

void mmu_unmap_region(void *va, size_t size) {
    uintptr_t v = (uintptr_t)va;
    uintptr_t end = PAGE_ALIGN(v + size);
    for (; v < end; v += PAGE_SIZE)
        mmu_unmap(kernel_pt.root, v);
}

void mmu_activate_user_space(PageTable *pt) {
    mmu_set_translation_base((pt->asid << 48) | (uintptr_t)pt->root);
}

void mmu_deactivate_user_space(void) {
    mmu_set_translation_base((uintptr_t)kernel_pt.root);
}

bool mmu_is_valid(uint64_t pte) {
    return pte & PTE_VALID;
}

void mmu_flush(void) {
    mmu_flush_tlb_local();
}

// --- Clone user space pagetable with COW and refcount ---
static void clone_pte(uintptr_t va, uint64_t pte, int level, void *arg) {
    if (level != 2)
        return;
    PageTable *dst_pt = (PageTable *)arg;
    uintptr_t pa = pte & PAGE_MASK;
    uint64_t attr = pte & ~PAGE_MASK;

    void *page = (void *)phys_to_virt(pa);
    incref(page);
    attr &= ~PTE_WRITE;
    mmu_map(dst_pt->root, va, pa, attr);
}

PageTable *mmu_clone_user_space(PageTable *src_pt) {
    PageTable *new_pt = malloc(sizeof(PageTable));
    if (!new_pt)
        return NULL;
    new_pt->root = alloc_page_table();
    if (!new_pt->root) {
        free(new_pt);
        return NULL;
    }
    new_pt->asid = 0;
    new_pt->valid = true;

    mmu_walk_all(src_pt->root, 0, 0, clone_pte, new_pt);
    return new_pt;
}

// --- Free user space page tables recursively ---
static void free_page_table(uint64_t *table, int level) {
    for (int i = 0; i < PTE_PER_TABLE; i++) {
        uint64_t pte = table[i];
        if (!(pte & PTE_VALID))
            continue;
        if ((pte & PTE_TABLE) && level < 2) {
            uint64_t *child = (uint64_t *)phys_to_virt(pte & PAGE_MASK);
            free_page_table(child, level + 1);
        } else if (level == 2) {
            void *page = (void *)phys_to_virt(pte & PAGE_MASK);
            decref(page);
        }
    }
    page_free(table);
}

void mmu_free_user_space(PageTable *pt) {
    if (!pt || !pt->root)
        return;
    free_page_table(pt->root, 0);
    pt->root = NULL;
    pt->valid = false;
    free(pt);
}

// --- Handle COW fault ---
void mmu_cow_fault_handler(PageTable *pt, uintptr_t fault_addr) {
    uint64_t *pte = mmuwalk(pt->root, fault_addr, 2, false);
    if (!pte || !(*pte & PTE_VALID))
        return; // invalid access

    uintptr_t old_pa = *pte & PAGE_MASK;
    void *old_page = (void *)phys_to_virt(old_pa);

    if (atomic_load(&((RefPage *)old_page)->refcount) > 1) {
        void *new_page = alloc_ref_page();
        if (!new_page)
            return;
        memcpy(new_page, old_page, PAGE_SIZE);
        decref(old_page);
        *pte = ((uintptr_t)new_page & PAGE_MASK) | (*pte & ~PAGE_MASK) | PTE_WRITE;
    } else {
        *pte |= PTE_WRITE;
    }

    mmu_flush_tlb_local();
}
