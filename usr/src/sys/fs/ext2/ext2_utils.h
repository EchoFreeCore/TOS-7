#ifndef EXT2_UTILS_H
#define EXT2_UTILS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

void *vfs_alloc_block_buffer(uint32_t blocksize);
void vfs_free_block_buffer(void *buf);

bool mmu_copy_from_user(void *dst, const void *src, size_t len);
bool mmu_copy_to_user(void *dst, const void *src, size_t len);

#endif
