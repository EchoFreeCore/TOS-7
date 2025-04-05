#include "mmu.h"
#include "pmm.h"
#include "process.h"
#include <errno.h>
#include <stddef.h>

void* sys_mmap(void* addr, size_t length, int prot, int flags, int fd, off_t offset) {
    (void)fd; (void)offset; // Ignoreras i denna version
    if (length == 0 || length > 0x10000000)
        return (void*)-EINVAL;

    uintptr_t va = current_proc->mmap_brk;
    uintptr_t end = (va + length + PAGE_SIZE - 1) & PAGE_MASK;

    for (uintptr_t ptr = va; ptr < end; ptr += PAGE_SIZE) {
        uintptr_t phys = pmm_alloc_page();
        if (!phys)
            return (void*)-ENOMEM;

        uint32_t flags = PAGE_PRESENT | PAGE_USER;
        if (prot & PROT_WRITE) flags |= PAGE_RW;
        vmm_map_user(current_proc->page_table, ptr, phys, flags);
    }

    current_proc->mmap_brk = end;
    return (void*)va;
}

int sys_munmap(void* addr, size_t length) {
    if (!addr || length == 0)
        return -EINVAL;

    uintptr_t va = (uintptr_t)addr;
    uintptr_t end = (va + length + PAGE_SIZE - 1) & PAGE_MASK;

    for (uintptr_t ptr = va; ptr < end; ptr += PAGE_SIZE) {
        uintptr_t phys = vmm_resolve(current_proc->page_table, ptr);
        if (phys) {
            vmm_unmap(current_proc->page_table, ptr);
            pmm_free_page(phys);
        }
    }

    return 0;
}