#include "vmm.h"
#include "pmm.h"
#include "panic.h"

void mmu_destroy_user_range(PageTable* pt, uintptr_t start, uintptr_t end) {
    if ((start % PAGE_SIZE) || (end % PAGE_SIZE) || start >= end)
        panic("mmu_destroy_user_range: invalid range");

    for (uintptr_t addr = start; addr < end; addr += PAGE_SIZE) {
        uintptr_t phys = virt_to_phys(pt->root, addr);
        if (phys) {
            mmu_unmap(pt->root, addr);
            pmm_free_page(phys);
        }
    }
}
