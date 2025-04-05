#include "vfs.h"
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define MAX_OPEN_FILES 128

static vnode_t *vfs_root = NULL;
static file_t *open_files[MAX_OPEN_FILES];

// Internal: resolve vnode from path
static vnode_t *resolve_path(const char *path) {
    if (!vfs_root || !path || *path == '\0') return NULL;

    vnode_t *current = vfs_root;
    char *path_copy = strdup(path);
    char *p = path_copy;
    if (*p == '/') p++;

    char *tok = strtok(p, "/");
    while (tok && current) {
        if (!current->ops || !current->ops->lookup) {
            current = NULL;
            break;
        }
        current = current->ops->lookup(current, tok);
        tok = strtok(NULL, "/");
    }

    free(path_copy);
    return current;
}

// Mount filesystem root
int vfs_mount_root(vnode_t *root) {
    vfs_root = root;
    return 0;
}

// Find free file descriptor
static int alloc_fd(file_t *file) {
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (!open_files[i]) {
            open_files[i] = file;
            return i;
        }
    }
    return -1;
}

// Free file descriptor
static void free_fd(int fd) {
    if (fd >= 0 && fd < MAX_OPEN_FILES)
        open_files[fd] = NULL;
}

// Open file (with O_CREAT support)
int vfs_open(const char *path, int flags, file_t **out) {
    vnode_t *node = resolve_path(path);

    // If doesn't exist but O_CREAT is set → create
    if (!node && (flags & O_CREAT)) {
        vnode_t *parent = resolve_path(dirname(strdup(path)));
        if (!parent || !parent->ops || !parent->ops->create)
            return -ENOENT;

        const char *name = strrchr(path, '/');
        name = name ? name + 1 : path;

        int rc = parent->ops->create(parent, name, VFS_FILE);
        if (rc != 0) return rc;

        node = parent->ops->lookup(parent, name);
        if (!node) return -EIO;
    }

    if (!node) return -ENOENT;

    if (node->ops && node->ops->open)
        node->ops->open(node);

    file_t *file = malloc(sizeof(file_t));
    file->node = node;
    file->offset = 0;
    file->flags = flags;
    file->refcount = 1;
    *out = file;
    return 0;
}

// Close file
int vfs_close(file_t *file) {
    if (!file) return -EINVAL;
    if (file->node && file->node->ops && file->node->ops->close)
        file->node->ops->close(file->node);
    free(file);
    return 0;
}

// Read
ssize_t vfs_read(file_t *file, void *buf, size_t len) {
    if (!file || !file->node || !file->node->ops || !file->node->ops->read)
        return -EINVAL;

    ssize_t ret = file->node->ops->read(file->node, buf, len, file->offset);
    if (ret > 0) file->offset += ret;
    return ret;
}

// Write
ssize_t vfs_write(file_t *file, const void *buf, size_t len) {
    if (!file || !file->node || !file->node->ops || !file->node->ops->write)
        return -EINVAL;

    ssize_t ret = file->node->ops->write(file->node, buf, len, file->offset);
    if (ret > 0) file->offset += ret;
    return ret;
}

// lseek support
int vfs_lseek(file_t *file, off_t offset, int whence) {
    if (!file) return -EINVAL;

    switch (whence) {
        case SEEK_SET: file->offset = offset; break;
        case SEEK_CUR: file->offset += offset; break;
        default: return -EINVAL;
    }

    return file->offset;
}

// stat by path
int vfs_stat(const char *path, struct stat *out) {
    vnode_t *node = resolve_path(path);
    if (!node || !node->ops || !node->ops->stat) return -ENOENT;
    return node->ops->stat(node, out);
}

// fstat
int vfs_fstat(file_t *file, struct stat *out) {
    if (!file || !file->node || !file->node->ops || !file->node->ops->stat)
        return -EINVAL;
    return file->node->ops->stat(file->node, out);
}

// readdir
int vfs_readdir(file_t *dir, int index, char *name_out) {
    if (!dir || !dir->node || !dir->node->ops || !dir->node->ops->readdir)
        return -EINVAL;
    return dir->node->ops->readdir(dir->node, index, name_out);
}

// mkdir
int vfs_mkdir(const char *path) {
    char *copy = strdup(path);
    char *slash = strrchr(copy, '/');
    if (!slash) return -EINVAL;

    *slash = '\0';
    const char *name = slash + 1;

    vnode_t *parent = resolve_path(copy);
    free(copy);

    if (!parent || !parent->ops || !parent->ops->mkdir)
        return -ENOTDIR;

    return parent->ops->mkdir(parent, name);
}

// unlink
int vfs_unlink(const char *path) {
    char *copy = strdup(path);
    char *slash = strrchr(copy, '/');
    if (!slash) return -EINVAL;

    *slash = '\0';
    const char *name = slash + 1;

    vnode_t *parent = resolve_path(copy);
    free(copy);

    if (!parent || !parent->ops || !parent->ops->unlink)
        return -ENOTDIR;

    return parent->ops->unlink(parent, name);
}

// rmdir
int vfs_rmdir(const char *path) {
    char *copy = strdup(path);
    char *slash = strrchr(copy, '/');
    if (!slash) return -EINVAL;

    *slash = '\0';
    const char *name = slash + 1;

    vnode_t *parent = resolve_path(copy);
    free(copy);

    if (!parent || !parent->ops || !parent->ops->rmdir)
        return -ENOTDIR;

    return parent->ops->rmdir(parent, name);
}

// truncate
int vfs_truncate(const char *path, off_t len) {
    vnode_t *node = resolve_path(path);
    if (!node || !node->ops || !node->ops->truncate) return -EINVAL;
    return node->ops->truncate(node, len);
}
