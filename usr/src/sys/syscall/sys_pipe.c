#include "pipe.h"
#include <errno.h>
#include <stddef.h>


int sys_pipe(int fds[2]) {
    pipe_t *p = pipe_create();
    if (!p) return -1;

    vnode_t *read_node = malloc(sizeof(vnode_t));
    vnode_t *write_node = malloc(sizeof(vnode_t));

    memset(read_node, 0, sizeof(vnode_t));
    memset(write_node, 0, sizeof(vnode_t));

    read_node->type = VFS_PIPE;
    write_node->type = VFS_PIPE;
    read_node->private_data = p;
    write_node->private_data = p;

    file_t *read_f = malloc(sizeof(file_t));
    file_t *write_f = malloc(sizeof(file_t));

    memset(read_f, 0, sizeof(file_t));
    memset(write_f, 0, sizeof(file_t));
    read_f->node = read_node;
    write_f->node = write_node;
    read_f->refcount = write_f->refcount = 1;

    file_t **fd_table = get_current_process()->fd_table;
    for (int i = 0; i < MAX_FD - 1; i++) {
        if (!fd_table[i] && !fd_table[i + 1]) {
            fd_table[i] = read_f;
            fd_table[i + 1] = write_f;
            fds[0] = i;
            fds[1] = i + 1;
            return 0;
        }
    }

    pipe_destroy(p);
    return -1;
}
