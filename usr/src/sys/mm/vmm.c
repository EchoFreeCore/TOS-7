#include "vmm.h"
#include "pmm.h"
#include "panic.h"

void vmm_destroy_user_range(pagetable_t pt, uintptr_t start, uintptr_t end) {
    if (start % PAGE_SIZE || end % PAGE_SIZE || start >= end)
        panic("vmm_destroy_user_range: invalid range");

    for (uintptr_t addr = start; addr < end; addr += PAGE_SIZE) {
        uintptr_t phys = vmm_resolve(pt, addr);
        if (phys) {
            vmm_unmap(pt, addr);
            pmm_free_page(phys);
        }
    }
}