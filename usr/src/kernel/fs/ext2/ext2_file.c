#include "ext2_file.h"
#include "ext2_inode.h"
#include "ext2_utils.h"
#include "ext2_block.h"
#include <string.h>

int ext2_read_file(const char *path, void *user_buf, device_t *dev, ext2_priv_data *priv) {
    inode_t inode;
    if (!ext2_find_file_inode(path, &inode, dev, priv)) return -1;

    uint8_t *buf = vfs_alloc_block_buffer(inode.size);
    if (!buf) return -1;

    uint32_t total = 0;
    for (int i = 0; i < 12 && total < inode.size; i++) {
        if (!inode.dbp[i]) break;
        void *blk = vfs_alloc_block_buffer(priv->blocksize);
        ext2_read_block(blk, inode.dbp[i], dev, priv);
        memcpy(buf + total, blk, priv->blocksize);
        total += priv->blocksize;
        vfs_free_block_buffer(blk);
    }

    mmu_copy_to_user(user_buf, buf, inode.size);
    vfs_free_block_buffer(buf);
    return 0;
}

int ext2_write_file(const char *path, const void *user_buf, uint32_t len, device_t *dev, ext2_priv_data *priv) {
    inode_t inode;
    uint32_t id = ext2_find_file_inode(path, &inode, dev, priv);
    if (!id) return -1;

    uint8_t *buf = vfs_alloc_block_buffer(len);
    if (!mmu_copy_from_user(buf, user_buf, len)) return -1;

    uint32_t total = 0;
    for (int i = 0; i < 12 && total < len; i++) {
        if (!inode.dbp[i]) ext2_alloc_block(&inode.dbp[i], dev, priv);

        void *blk = vfs_alloc_block_buffer(priv->blocksize);
        memcpy(blk, buf + total, priv->blocksize);
        ext2_write_block(blk, inode.dbp[i], dev, priv);
        total += priv->blocksize;
        vfs_free_block_buffer(blk);
    }

    inode.size = total;
    ext2_write_inode(&inode, id, dev, priv);
    vfs_free_block_buffer(buf);
    return 0;
}
