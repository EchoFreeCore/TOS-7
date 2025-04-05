#include "ext2_dir.h"
#include "ext2_inode.h"
#include "ext2_utils.h"
#include "ext2_block.h"
#include <string.h>

int ext2_read_dir(const char *path, char *buf, device_t *dev, ext2_priv_data *priv) {
    inode_t dir_inode;
    if (!ext2_find_file_inode(path, &dir_inode, dev, priv)) return -1;

    size_t offset = 0;
    for (int i = 0; i < 12; i++) {
        if (!dir_inode.dbp[i]) break;
        void *blk = vfs_alloc_block_buffer(priv->blocksize);
        ext2_read_block(blk, dir_inode.dbp[i], dev, priv);

        ext2_dir *entry = (ext2_dir *)blk;
        while (entry->inode != 0) {
            memcpy(buf + offset, ((char *)&entry->reserved) + 1, entry->namelength);
            offset += entry->namelength;
            buf[offset++] = '\n';
            entry = (ext2_dir *)((uint8_t *)entry + entry->size);
        }

        vfs_free_block_buffer(blk);
    }

    buf[offset] = '\0';
    return 0;
}
