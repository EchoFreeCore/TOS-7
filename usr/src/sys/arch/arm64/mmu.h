#ifndef MMU_H
#define MMU_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define PAGE_SIZE       4096
#define PAGE_SHIFT      12
#define PAGE_MASK       (~(PAGE_SIZE - 1))
#define PAGE_ALIGN(x)   (((x) + PAGE_SIZE - 1) & PAGE_MASK)

#define PT_LEVELS       3
#define PTE_PER_TABLE   512
#define LEVEL_SHIFT(lvl) (PAGE_SHIFT + ((PT_LEVELS - 1 - (lvl)) * 9))
#define PTE_INDEX(va, lvl) (((va) >> LEVEL_SHIFT(lvl)) & 0x1FF)

#define PTE_VALID       (1ULL << 0)
#define PTE_TABLE       (1ULL << 1)
#define PTE_WRITE       (1ULL << 10)
#define PTE_USER        (1ULL << 6)
#define PTE_COW         (1ULL << 55)  // Software-defined bit for COW

// Simple refcounted page descriptor
typedef struct RefPage {
    void *page;
    int refcount;
} RefPage;

// Top-level page table structure
typedef struct PageTable {
    uint64_t *root;  // L1 table
    uint64_t asid;
    bool valid;
} PageTable;

// MMU core APIs
void mmu_init(void);
void mmu_map(uint64_t *l1_table, uintptr_t va, uintptr_t pa, uint64_t attr);
void mmu_unmap(uint64_t *l1_table, uintptr_t va);
uintptr_t virt_to_phys(uint64_t *l1_table, uintptr_t va);
void mmu_map_identity(uintptr_t phys, uintptr_t size, uint64_t attr_flags);
void mmu_assign_asid(PageTable *pt, uint64_t asid);
void *mmu_map_region(uintptr_t phys, size_t size, uint64_t flags);
void mmu_unmap_region(void *va, size_t size);
void mmu_activate_user_space(PageTable *pt);
void mmu_deactivate_user_space(void);
bool mmu_is_valid(uint64_t pte);
void mmu_flush(void);

// Page table management
uint64_t *mmuwalk(uint64_t *root, uintptr_t va, int target_level, bool alloc);
void mmu_walk_all(uint64_t *root, int level, uintptr_t base_va,
                  void (*callback)(uintptr_t va, uint64_t pte, int level, void *arg),
                  void *arg);

// User address space management
PageTable *mmu_clone_user_space(PageTable *src);
void mmu_free_user_space(PageTable *pt);
void mmu_cow_fault_handler(PageTable *pt, uintptr_t fault_addr);

#endif // MMU_H