#ifndef EXT2_DIR_H
#define EXT2_DIR_H

#include "device.h"
#include "ext2_types.h"

int ext2_read_dir(const char *path, char *buf, device_t *dev, ext2_priv_data *priv);

#endif
