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