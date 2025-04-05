#ifndef EXT2_INODE_H
#define EXT2_INODE_H

#include <stdint.h>
#include "device.h"
#include "ext2_types.h"

int ext2_read_inode(inode_t *out, uint32_t inode_num, device_t *dev, ext2_priv_data *priv);
int ext2_write_inode(const inode_t *in, uint32_t inode_num, device_t *dev, ext2_priv_data *priv);
uint32_t ext2_find_file_inode(const char *path, inode_t *out, device_t *dev, ext2_priv_data *priv);

#endif
