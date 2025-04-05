#include "process.h"
#include "thread.h"
#include "scheduler.h"
#include "vfs.h"
#include "kmalloc.h"

long sys_fork(void) {
    process_t *parent = process_current();
    if (!parent) return -1;

    process_t *child = process_create(parent->pid);
    if (!child) return -1;

    // Duplicera fd_table (öka refcount på vnodes)
    for (int i = 0; i < MAX_FDS; ++i) {
        if (parent->fd_table[i]) {
            child->fd_table[i] = parent->fd_table[i];
            vfs_ref(child->fd_table[i]);
        }
    }

    child->cwd = parent->cwd;
    if (child->cwd) vfs_ref(child->cwd);

    // Skapa ny tråd
    thread_t *t = thread_clone(current_thread[this_cpu()]);
    if (!t) {
        process_destroy(child);
        return -1;
    }

    t->process = child;
    child->main_thread = t;

    // Sätt barnets return-värde till 0
    t->fork_return_value = 0;

    scheduler_enqueue(this_cpu(), t);

    return child->pid; // Förälder får pid
}
