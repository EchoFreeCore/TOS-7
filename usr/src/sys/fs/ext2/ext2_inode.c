#include "ext2_inode.h"
#include "ext2_utils.h"
#include "ext2_block.h"
#include <string.h>

int ext2_read_inode(inode_t *out, uint32_t inode_num, device_t *dev, ext2_priv_data *priv) {
    void *buf = vfs_alloc_block_buffer(priv->blocksize);
    if (!buf) return -1;

    uint32_t bg = (inode_num - 1) / priv->sb.inodes_in_blockgroup;
    uint32_t index = (inode_num - 1) % priv->sb.inodes_in_blockgroup;
    uint32_t bgd_block = priv->first_bgd + bg;

    ext2_read_block(buf, bgd_block, dev, priv);
    block_group_desc_t *bgd = (block_group_desc_t *)buf;

    uint32_t inode_table_block = bgd->block_of_inode_table;
    uint32_t block = inode_table_block + ((index * sizeof(inode_t)) / priv->blocksize);
    ext2_read_block(buf, block, dev, priv);

    inode_t *arr = (inode_t *)buf;
    memcpy(out, &arr[index % priv->inodes_per_block], sizeof(inode_t));
    vfs_free_block_buffer(buf);
    return 0;
}

int ext2_write_inode(const inode_t *in, uint32_t inode_num, device_t *dev, ext2_priv_data *priv) {
    void *buf = vfs_alloc_block_buffer(priv->blocksize);
    if (!buf) return -1;

    uint32_t bg = (inode_num - 1) / priv->sb.inodes_in_blockgroup;
    uint32_t index = (inode_num - 1) % priv->sb.inodes_in_blockgroup;
    uint32_t bgd_block = priv->first_bgd + bg;

    ext2_read_block(buf, bgd_block, dev, priv);
    block_group_desc_t *bgd = (block_group_desc_t *)buf;

    uint32_t inode_table_block = bgd->block_of_inode_table;
    uint32_t block = inode_table_block + ((index * sizeof(inode_t)) / priv->blocksize);
    ext2_read_block(buf, block, dev, priv);

    inode_t *arr = (inode_t *)buf;
    memcpy(&arr[index % priv->inodes_per_block], in, sizeof(inode_t));
    ext2_write_block(buf, block, dev, priv);
    vfs_free_block_buffer(buf);
    return 0;
}
