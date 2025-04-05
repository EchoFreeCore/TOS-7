#ifndef VFS_H
#define VFS_H

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>

typedef enum {
    VFS_FILE,
    VFS_DIR,
    VFS_CHARDEV,
    VFS_BLOCKDEV,
    VFS_SYMLINK,
    VFS_PIPE,
} vnode_type_t;

struct vnode;

typedef struct vfs_ops {
    ssize_t (*read)(struct vnode *, void *buf, size_t len, off_t off);
    ssize_t (*write)(struct vnode *, const void *buf, size_t len, off_t off);
    int     (*open)(struct vnode *);
    int     (*close)(struct vnode *);
    struct vnode* (*lookup)(struct vnode *, const char *name);
    int     (*create)(struct vnode *, const char *name, vnode_type_t type);
    int     (*unlink)(struct vnode *, const char *name);
    int     (*mkdir)(struct vnode *, const char *name);
    int     (*rmdir)(struct vnode *, const char *name);
    int     (*readdir)(struct vnode *, int index, char *name_out);
    int     (*stat)(struct vnode *, struct stat *out);
    int     (*truncate)(struct vnode *, off_t len);
} vfs_ops_t;

typedef struct vnode {
    vnode_type_t type;
    vfs_ops_t *ops;
    void *private_data;
    struct vnode *parent;
    char name[64];
} vnode_t;

// Öppen filbeskrivare
typedef struct file {
    vnode_t *node;
    off_t offset;
    int flags;
    int refcount;
} file_t;

// Public VFS API
int vfs_mount_root(vnode_t *root);

int vfs_open(const char *path, int flags, file_t **out);
int vfs_close(file_t *file);
ssize_t vfs_read(file_t *file, void *buf, size_t len);
ssize_t vfs_write(file_t *file, const void *buf, size_t len);
int vfs_stat(const char *path, struct stat *out);
int vfs_fstat(file_t *file, struct stat *out);
int vfs_lseek(file_t *file, off_t offset, int whence);

int vfs_mkdir(const char *path);
int vfs_unlink(const char *path);
int vfs_rmdir(const char *path);
int vfs_truncate(const char *path, off_t len);
int vfs_readdir(file_t *dir, int index, char *name_out);

#endif



