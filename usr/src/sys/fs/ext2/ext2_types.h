#ifndef EXT2_TYPES_H
#define EXT2_TYPES_H

#include <stdint.h>

#define EXT2_SIGNATURE 0xEF53
#define INODE_TYPE_FILE 0x8000
#define INODE_TYPE_DIRECTORY 0x4000

typedef struct {
    uint32_t blocks;
    uint32_t inodes;
    uint32_t blocks_in_blockgroup;
    uint32_t inodes_in_blockgroup;
    uint32_t unallocatedblocks;
    uint32_t unallocatedinodes;
    uint32_t superblock_id;
    uint8_t blocksize_hint;
    uint16_t ext2_sig;
} superblock_t;

typedef struct {
    uint32_t block_of_inode_table;
    uint16_t num_of_unalloc_block;
    uint16_t num_of_unalloc_inode;
} block_group_desc_t;

typedef struct {
    uint16_t type;
    uint32_t size;
    uint32_t dbp[14];
} inode_t;

typedef struct {
    uint32_t inode;
    uint16_t size;
    uint8_t namelength;
    uint8_t reserved;
} ext2_dir;

typedef struct {
    superblock_t sb;
    uint32_t blocksize;
    uint32_t inodes_per_block;
    uint32_t sectors_per_block;
    uint32_t number_of_bgs;
    uint32_t first_bgd;
} ext2_priv_data;

#endif
