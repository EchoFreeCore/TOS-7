#include "vfs.h"
#include "syscalls.h"
#include "spinlock.h"
#include "process_context.h" // get_current_process()

int sys_open(const char *path, int flags) {
    file_t *f = NULL;
    if (vfs_open(path, flags, &f) != 0) return -1;

    spinlock_init(&f->lock);
    f->refcount = 1;

    file_t **fd_table = get_current_process()->fd_table;

    for (int fd = 0; fd < MAX_FD; fd++) {
        if (!fd_table[fd]) {
            fd_table[fd] = f;
            return fd;
        }
    }

    vfs_close(f);
    return -1;
}

int sys_close(int fd) {
    if (fd < 0 || fd >= MAX_FD) return -1;

    file_t **fd_table = get_current_process()->fd_table;
    file_t *f = fd_table[fd];
    if (!f) return -1;

    spin_lock(&f->lock);
    f->refcount--;
    bool last = (f->refcount == 0);
    spin_unlock(&f->lock);

    fd_table[fd] = NULL;
    if (last) vfs_close(f);
    return 0;
}

ssize_t sys_read(int fd, void *buf, size_t len) {
    if (fd < 0 || fd >= MAX_FD) return -1;
    file_t *f = get_current_process()->fd_table[fd];
    if (!f) return -1;

    spin_lock(&f->lock);
    ssize_t ret = vfs_read(f, buf, len);
    spin_unlock(&f->lock);
    return ret;
}

ssize_t sys_write(int fd, const void *buf, size_t len) {
    if (fd < 0 || fd >= MAX_FD) return -1;
    file_t *f = get_current_process()->fd_table[fd];
    if (!f) return -1;

    spin_lock(&f->lock);
    ssize_t ret = vfs_write(f, buf, len);
    spin_unlock(&f->lock);
    return ret;
}

off_t sys_lseek(int fd, off_t offset, int whence) {
    if (fd < 0 || fd >= MAX_FD) return -1;
    file_t *f = get_current_process()->fd_table[fd];
    if (!f) return -1;

    spin_lock(&f->lock);
    int ret = vfs_lseek(f, offset, whence);
    spin_unlock(&f->lock);
    return ret;
}

int sys_stat(const char *path, struct stat *out) {
    return vfs_stat(path, out);
}

int sys_fstat(int fd, struct stat *out) {
    if (fd < 0 || fd >= MAX_FD) return -1;
    file_t *f = get_current_process()->fd_table[fd];
    if (!f) return -1;

    spin_lock(&f->lock);
    int ret = vfs_fstat(f, out);
    spin_unlock(&f->lock);
    return ret;
}

int sys_mkdir(const char *path) {
    return vfs_mkdir(path);
}

int sys_unlink(const char *path) {
    return vfs_unlink(path);
}

int sys_rmdir(const char *path) {
    return vfs_rmdir(path);
}

int sys_truncate(const char *path, off_t len) {
    return vfs_truncate(path, len);
}

int sys_dup(int oldfd) {
    if (oldfd < 0 || oldfd >= MAX_FD) return -1;
    file_t *f = get_current_process()->fd_table[oldfd];
    if (!f) return -1;

    spin_lock(&f->lock);
    f->refcount++;
    spin_unlock(&f->lock);

    file_t **fd_table = get_current_process()->fd_table;

    for (int fd = 0; fd < MAX_FD; fd++) {
        if (!fd_table[fd]) {
            fd_table[fd] = f;
            return fd;
        }
    }

    sys_close(oldfd); // release extra ref
    return -1;
}

int sys_dup2(int oldfd, int newfd) {
    if (oldfd < 0 || newfd < 0 || oldfd >= MAX_FD || newfd >= MAX_FD)
        return -1;

    file_t **fd_table = get_current_process()->fd_table;
    file_t *f = fd_table[oldfd];
    if (!f) return -1;

    if (oldfd == newfd) return newfd;

    if (fd_table[newfd]) sys_close(newfd);

    spin_lock(&f->lock);
    f->refcount++;
    spin_unlock(&f->lock);

    fd_table[newfd] = f;
    return newfd;
}
