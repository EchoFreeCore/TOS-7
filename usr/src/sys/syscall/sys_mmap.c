#include "mmu.h"
#include "pmm.h"
#include "process.h"
#include <errno.h>
#include <stddef.h>
#include <stdint.h>

void* sys_mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset) {
    (void)fd; (void)offset; // MAP_ANON only

    if (length == 0 || length > 0x10000000)
        return (void*)-EINVAL;

    // Justera till pagesize
    size_t size = PAGE_ALIGN(length);

    // Allokera fysisk region (för enkelhet: en contiguous area)
    uintptr_t phys_base = pmm_alloc_pages(size / PAGE_SIZE);
    if (!phys_base)
        return (void*)-ENOMEM;

    // Översätt prot till MMU-flaggor
    uint64_t mmu_flags = PTE_VALID | PTE_USER;
    if (prot & PROT_WRITE) mmu_flags |= PTE_WRITE;

    // Välj virtuell adress i mmap-regionen
    uintptr_t va = current_proc->mmap_brk;
    void* virt = mmu_map_region(phys_base, size, mmu_flags);
    if (!virt)
        return (void*)-ENOMEM;

    // Uppdatera brk för framtida anrop
    current_proc->mmap_brk = (uintptr_t)virt + size;

    return virt;
}

int sys_munmap(void* addr, size_t length) {
    if (!addr || length == 0)
        return -EINVAL;

    mmu_unmap_region(addr, PAGE_ALIGN(length));
    return 0;
}
