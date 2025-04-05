#ifndef EXT2_ALLOC_H
#define EXT2_ALLOC_H

#include <stdint.h>
#include "device.h"
#include "ext2_types.h"

int ext2_alloc_inode(uint32_t *out, device_t *dev, ext2_priv_data *priv);
int ext2_alloc_block(uint32_t *out, device_t *dev, ext2_priv_data *priv);

#endif
