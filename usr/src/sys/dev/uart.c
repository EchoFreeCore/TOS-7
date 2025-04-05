static ssize_t uart0_read(vnode_t* vn, void* buf, size_t len, off_t off);
static ssize_t uart0_write(vnode_t* vn, const void* buf, size_t len, off_t off);

static vnode_ops_t uart0_data_ops = {
    .read = uart0_read,
    .write = uart0_write
};

vnode_t* uart0_get_node(const char* subpath) {
    if (strcmp(subpath, "data") == 0) {
        static vnode_t v;
        v.ops = &uart0_data_ops;
        v.type = VFS_CHARDEV;
        v.private_data = &uart0_hw_state;
        return &v;
    }
    return NULL;
}

static dev_entry_t uart0_dev = {
    .name = "uart0",
    .get_node = uart0_get_node
};