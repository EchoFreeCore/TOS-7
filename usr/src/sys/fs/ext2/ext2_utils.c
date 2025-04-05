#include "ext2_utils.h"
#include <stdlib.h>
#include <string.h>

void *vfs_alloc_block_buffer(uint32_t blocksize) {
    void *ptr = NULL;
    posix_memalign(&ptr, 4096, blocksize);
    return ptr;
}

void vfs_free_block_buffer(void *buf) {
    free(buf);
}

bool mmu_copy_from_user(void *dst, const void *src, size_t len) {
    if (!dst || !src || len == 0) return false;
    memcpy(dst, src, len);
    return true;
}

bool mmu_copy_to_user(void *dst, const void *src, size_t len) {
    if (!dst || !src || len == 0) return false;
    memcpy(dst, src, len);
    return true;
}
