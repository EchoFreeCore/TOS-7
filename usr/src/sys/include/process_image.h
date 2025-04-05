#ifndef PROCESS_IMAGE_H
#define PROCESS_IMAGE_H

#include <stdint.h>
#include <stddef.h>

typedef struct process_image {
    void* entry_point;
    void* stack_top;
    uintptr_t heap_base;
    uintptr_t heap_end;
    uintptr_t user_base;
    uintptr_t user_top;
    int argc;
    char** argv;
    char** envp;
} process_image_t;

#endif