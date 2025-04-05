#include "process.h"
#include "kmalloc.h"
#include <string.h>

process_t *process_table[MAX_PROCESSES];
static pid_t next_pid = 1;

process_t *process_create(pid_t ppid) {
    process_t *p = kcalloc(1, sizeof(process_t));
    if (!p) return NULL;

    p->pid = next_pid++;
    p->ppid = ppid;
    p->uid = 0;
    p->gid = 0;

    process_table[p->pid] = p;
    return p;
}

void process_destroy(process_t *proc) {
    if (!proc) return;
    for (int i = 0; i < MAX_FDS; ++i) {
        if (proc->fd_table[i]) {
            vfs_close(proc->fd_table[i]);
        }
    }
    process_table[proc->pid] = NULL;
    kfree(proc);
}

process_t *process_lookup(pid_t pid) {
    if (pid < 0 || pid >= MAX_PROCESSES) return NULL;
    return process_table[pid];
}

process_t *process_current(void) {
    cpu_id_t cpu = this_cpu();
    thread_t *t = current_thread[cpu];
    return t ? t->process : NULL;
}
