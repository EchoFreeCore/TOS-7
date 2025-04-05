#include "mmu.h"
#include "arch/arm64/mmu.h"
#include "pmm.h"
#include <string.h>

static pagetable_t * kernel_l1;

typedef uint64_t pte_t;
typedef pte_t* pagetable_t;

extern pagetable_t kernel_l1;

void mmu_init(void) {
    kernel_l1 = (pagetable_t)pmm_alloc_page();
    memset(kernel_l1, 0, PAGE_SIZE);
   
    // Identity-map kernel text/data
    for (uintptr_t addr = 0x80000; addr < 0x200000; addr += PAGE_SIZE) {
        vmm_map_kernel(addr, addr, PAGE_PRESENT | PAGE_RW);
    }
   
    mmu_enable((uintptr_t)kernel_l1);
}

void mmu_enable(pagetable_t pt) {
    arch_mmu_write_ttbr0(0); // tom för userland
    arch_mmu_write_ttbr1((uintptr_t)pt);
    arch_mmu_setup();
}


static pte_t make_block_entry(uintptr_t phys, uint32_t flags) {
    pte_t entry = PAGE_ALIGN_DOWN(phys) | PTE_VALID | PTE_AF | PTE_SH_INNER | PTE_ATTR_IDX(ATTR_IDX_NORMAL);

    if (!(flags & PAGE_RW)) entry |= PTE_RO;
    if (!(flags & PAGE_USER)) entry |= PTE_PXN | PTE_UXN;

    return entry;
}

static pagetable_t get_next_level(pagetable_t table, size_t index, int level) {
    if (table[index] & PTE_VALID) {
        return (pagetable_t)(table[index] & PAGE_MASK);
    }

    uintptr_t new_table_phys = pmm_alloc_page();
    if (!new_table_phys) return NULL;

    memset((void*)new_table_phys, 0, PAGE_SIZE);
    table[index] = new_table_phys | PTE_VALID | PTE_TABLE;

    return (pagetable_t)new_table_phys;
}

static pagetable_t ensure_pagetable(pagetable_t root, uintptr_t va, int alloc_user) {
    pagetable_t l1 = root;
    pagetable_t l2 = get_next_level(l1, L1_INDEX(va), 1);
    if (!l2) return NULL;

    pagetable_t l3 = get_next_level(l2, L2_INDEX(va), 2);
    if (!l3) return NULL;

    return l3;
}

void vmm_map_kernel(uintptr_t va, uintptr_t pa, uint32_t flags) {
    pagetable_t l3 = ensure_pagetable(kernel_l1, va, 0);
    if (!l3) return;

    size_t idx = L3_INDEX(va);
    l3[idx] = make_block_entry(pa, flags);
}

void vmm_map_user(pagetable_t root, uintptr_t va, uintptr_t pa, uint32_t flags) {
    pagetable_t l3 = ensure_pagetable(root, va, 1);
    if (!l3) return;

    size_t idx = L3_INDEX(va);
    l3[idx] = make_block_entry(pa, flags | PAGE_USER);
}

uintptr_t vmm_resolve(pagetable_t root, uintptr_t va) {
    pagetable_t l1 = root;
    if (!(l1[L1_INDEX(va)] & PTE_VALID)) return 0;

    pagetable_t l2 = (pagetable_t)(l1[L1_INDEX(va)] & PAGE_MASK);
    if (!(l2[L2_INDEX(va)] & PTE_VALID)) return 0;

    pagetable_t l3 = (pagetable_t)(l2[L2_INDEX(va)] & PAGE_MASK);
    if (!(l3[L3_INDEX(va)] & PTE_VALID)) return 0;

    return l3[L3_INDEX(va)] & PAGE_MASK;
}

void vmm_unmap(pagetable_t root, uintptr_t va) {
    pagetable_t l1 = root;
    if (!(l1[L1_INDEX(va)] & PTE_VALID)) return;

    pagetable_t l2 = (pagetable_t)(l1[L1_INDEX(va)] & PAGE_MASK);
    if (!(l2[L2_INDEX(va)] & PTE_VALID)) return;

    pagetable_t l3 = (pagetable_t)(l2[L2_INDEX(va)] & PAGE_MASK);
    l3[L3_INDEX(va)] = 0;
}

int vmm_is_user_mapped(pagetable_t root, uintptr_t va, size_t len) {
    for (size_t offset = 0; offset < len; offset += PAGE_SIZE) {
        if (!vmm_resolve(root, va + offset))
            return 0;
    }
    return 1;
}

void vmm_flush(pagetable_t pt) {
    (void)pt;
    asm volatile("dsb ish; isb");
}
