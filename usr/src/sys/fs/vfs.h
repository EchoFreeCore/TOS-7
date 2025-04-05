typedef struct vnode {
    enum vfs_type type;
    struct vnode_ops* ops;
    void* private_data;
} vnode_t;

typedef struct vnode_ops {
    ssize_t(*read)(vnode_t*, void*, size_t, off_t);
    ssize_t(*write)(vnode_t*, const void*, size_t, off_t);
    int (*open)(vnode_t*);
    int (*close)(vnode_t*);
    int (*lookup)(vnode_t*, const char*, vnode_t*);
    int (*readdir)(vnode_t*, int, char*);
    int (*stat)(vnode_t*, struct stat*);
    int (*mkdir)(vnode_t*, const char*, int);
    int (*unlink)(vnode_t*, const char*);
} vfs_ops_t;