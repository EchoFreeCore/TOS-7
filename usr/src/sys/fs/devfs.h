#ifndef DEVFS_H
#define DEVFS_H
#include "vfs.h"

typedef enum { DEV_CHAR, DEV_DIR } dev_type_t;

typedef struct devfs_entry {
    const char* name;
    dev_type_t type;
    struct vnode_ops* ops;
    void* private_data;
    struct devfs_entry* children;
    struct devfs_entry* next;
} devfs_entry_t;

int devfs_mount(vnode_t* out_root);
void devfs_register(devfs_entry_t* entry);
#endif

#include "devfs.h"
#include <string.h>
#include <errno.h>

// Intern root
static devfs_entry_t* devfs_root = NULL;

// === VFS Operationer ===

static int devfs_lookup(vnode_t* dir, const char* name, vnode_t* out) {
    devfs_entry_t* parent = (devfs_entry_t*)dir->private_data;
    devfs_entry_t* cur = parent->children;

    while (cur) {
        if (strcmp(cur->name, name) == 0) {
            memset(out, 0, sizeof(vnode_t));
            out->type = (cur->type == DEV_DIR) ? VFS_DIR : VFS_CHARDEV;
            out->ops = cur->ops;
            out->private_data = cur->private_data;
            return 0;
        }
        cur = cur->next;
    }

    return -ENOENT;
}

static int devfs_stat(vnode_t* vn, struct stat* st) {
    // Enkel stat – kan utökas
    memset(st, 0, sizeof(struct stat));
    st->st_mode = S_IFCHR | 0666;
    return 0;
}

static vfs_ops_t devfs_dir_ops = {
    .lookup = devfs_lookup,
    .stat = devfs_stat
};

// === Montering ===

int devfs_mount(vnode_t* out_root) {
    static vnode_t root_vnode;

    memset(&root_vnode, 0, sizeof(vnode_t));
    root_vnode.type = VFS_DIR;
    root_vnode.ops = &devfs_dir_ops;
    root_vnode.private_data = devfs_root;

    *out_root = root_vnode;
    return 0;
}

// === Registrering ===

void devfs_register(devfs_entry_t* entry) {
    // Lägg till entry i rootens länkade lista
    entry->next = devfs_root;
    devfs_root = entry;
}



