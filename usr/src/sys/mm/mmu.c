#include "mmu.h"
#include "pmm.h"
#include <string.h>

// Här kan du hantera page table traversal på ett generiskt sätt,
// men den faktiska mappningen, registern och attribut sätts i arch/arm64/mmu.c

void* mmu_alloc_pagetable(void) {
    uintptr_t phys = pmm_alloc_page();
    if (!phys) return NULL;
    memset((void*)phys, 0, PAGE_SIZE);
    return (void*)phys;
}

// Stubbar som anropar arkitekturberoende implementation
__attribute__((weak)) void mmu_enable(pagetable_t kernel_pt) { (void)kernel_pt; }

__attribute__((weak)) void vmm_map_kernel(uintptr_t v, uintptr_t p, uint32_t flags) {
    (void)v; (void)p; (void)flags;
}

__attribute__((weak)) void vmm_map_user(pagetable_t pt, uintptr_t v, uintptr_t p, uint32_t flags) {
    (void)pt; (void)v; (void)p; (void)flags;
}

__attribute__((weak)) uintptr_t vmm_resolve(pagetable_t pt, uintptr_t virt) {
    (void)pt; (void)virt;
    return 0;
}

__attribute__((weak)) void vmm_unmap(pagetable_t pt, uintptr_t vaddr) {
    (void)pt; (void)vaddr;
}

__attribute__((weak)) int vmm_is_user_mapped(pagetable_t pt, uintptr_t vaddr, size_t len) {
    (void)pt; (void)vaddr; (void)len;
    return 1;
}

__attribute__((weak)) void vmm_flush(pagetable_t pt) {
    (void)pt;
}
