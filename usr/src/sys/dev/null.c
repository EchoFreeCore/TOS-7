#include "devfs.h"
#include <stddef.h>

static ssize_t dev_null_read(vnode_t* vn, void* buf, size_t len, off_t off) {
    return 0; // alltid EOF
}

static ssize_t dev_null_write(vnode_t* vn, const void* buf, size_t len, off_t off) {
    return len; // släng allt
}

static vnode_ops_t null_ops = {
    .read = dev_null_read,
    .write = dev_null_write
};

static devfs_entry_t dev_null = {
    .name = "null",
    .type = DEV_CHAR,
    .ops = &null_ops,
    .private_data = NULL,
    .children = NULL,
    .next = NULL
};

void dev_null_init() {
    devfs_register(&dev_null);
}