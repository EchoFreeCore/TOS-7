#ifndef MMU_H
#define MMU_H

#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE   0x1000
#define PAGE_MASK   (~(PAGE_SIZE - 1))

#define PAGE_PRESENT (1 << 0)
#define PAGE_RW      (1 << 1)
#define PAGE_USER    (1 << 2)

typedef uint64_t pagetable_t;

void mmu_init(void);
void mmu_enable(pagetable_t kernel_pt);

void* mmu_alloc_pagetable(void);

void vmm_map_kernel(uintptr_t virt, uintptr_t phys, uint32_t flags);
void vmm_map_user(pagetable_t pt, uintptr_t virt, uintptr_t phys, uint32_t flags);
uintptr_t vmm_resolve(pagetable_t pt, uintptr_t virt);
void vmm_unmap(pagetable_t pt, uintptr_t virt);
int vmm_is_user_mapped(pagetable_t pt, uintptr_t virt, size_t len);
void vmm_flush(pagetable_t pt);

#endif
