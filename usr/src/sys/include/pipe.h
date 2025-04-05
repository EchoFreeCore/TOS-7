// pipe.h
#ifndef PIPE_H
#define PIPE_H

#include <stdint.h>
#include <stddef.h>
#include "spinlock.h"

#define PIPE_BUF_SIZE 4096

typedef struct pipe {
    uint8_t buffer[PIPE_BUF_SIZE];
    size_t read_pos;
    size_t write_pos;
    size_t count;
    spinlock_t lock;
} pipe_t;

pipe_t *pipe_create(void);
void pipe_destroy(pipe_t *p);
ssize_t pipe_read(pipe_t *p, void *buf, size_t len);
ssize_t pipe_write(pipe_t *p, const void *buf, size_t len);

#endif
