#include "pipe.h"
#include <stdlib.h>
#include <string.h>

pipe_t *pipe_create(void) {
    pipe_t *p = malloc(sizeof(pipe_t));
    if (!p) return NULL;
    memset(p, 0, sizeof(pipe_t));
    spinlock_init(&p->lock);
    return p;
}

void pipe_destroy(pipe_t *p) {
    if (p) free(p);
}

ssize_t pipe_read(pipe_t *p, void *buf, size_t len) {
    spin_lock(&p->lock);
    size_t r = 0;
    while (r < len && p->count > 0) {
        ((uint8_t *)buf)[r++] = p->buffer[p->read_pos++];
        p->read_pos %= PIPE_BUF_SIZE;
        p->count--;
    }
    spin_unlock(&p->lock);
    return r;
}

ssize_t pipe_write(pipe_t *p, const void *buf, size_t len) {
    spin_lock(&p->lock);
    size_t w = 0;
    while (w < len && p->count < PIPE_BUF_SIZE) {
        p->buffer[p->write_pos++] = ((uint8_t *)buf)[w++];
        p->write_pos %= PIPE_BUF_SIZE;
        p->count++;
    }
    spin_unlock(&p->lock);
    return w;
}
