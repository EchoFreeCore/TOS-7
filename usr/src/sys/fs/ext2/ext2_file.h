#ifndef EXT2_FILE_H
#define EXT2_FILE_H

#include <stdint.h>
#include "device.h"
#include "ext2_types.h"

int ext2_read_file(const char *path, void *user_buf, device_t *dev, ext2_priv_data *priv);
int ext2_write_file(const char *path, const void *user_buf, uint32_t len, device_t *dev, ext2_priv_data *priv);

#endif
