#include "ext2_alloc.h"
#include "ext2_utils.h"
#include "ext2_block.h"

int ext2_alloc_inode(uint32_t *out, device_t *dev, ext2_priv_data *priv) {
    *out = 1;
    return 0;
}

int ext2_alloc_block(uint32_t *out, device_t *dev, ext2_priv_data *priv) {
    static uint32_t next = 100;
    *out = next++;
    return 0;
}
