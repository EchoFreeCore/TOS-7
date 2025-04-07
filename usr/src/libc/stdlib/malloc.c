#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>

#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))
#define BLOCK_OVERHEAD (sizeof(struct block_header))
#define ARENA_SIZE (128 * 1024) // 128KB

typedef struct block_header {
    size_t size;
    int free;
    struct block_header* next;
} block_header;

static block_header* free_list = NULL;

// Forward declarations
void* malloc(size_t size);
void free(void* ptr);
void* realloc(void* ptr, size_t size);
void* calloc(size_t nmemb, size_t size);

static void* request_memory_from_os(size_t size) {
    void* ptr = mmap(0, size, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANON, -1, 0);
    if (ptr == (void*)-1) return NULL;
    return ptr;
}

static block_header* find_free_block(size_t size) {
    block_header* curr = free_list;
    while (curr) {
        if (curr->free && curr->size >= size) return curr;
        curr = curr->next;
    }
    return NULL;
}

static void split_block(block_header* block, size_t size) {
    if (block->size >= size + BLOCK_OVERHEAD + ALIGNMENT) {
        block_header* new_block = (block_header*)((char*)block + BLOCK_OVERHEAD + size);
        new_block->size = block->size - size - BLOCK_OVERHEAD;
        new_block->free = 1;
        new_block->next = block->next;

        block->size = size;
        block->next = new_block;
    }
}

void* malloc(size_t size) {
    if (size == 0) return NULL;
    size = ALIGN(size);

    block_header* block = find_free_block(size);
    if (block) {
        split_block(block, size);
        block->free = 0;
        return (void*)(block + 1);
    }

    size_t total_size = size + BLOCK_OVERHEAD;
    size_t arena_size = (total_size < ARENA_SIZE) ? ARENA_SIZE : total_size;

    void* arena = request_memory_from_os(arena_size);
    if (!arena) return NULL;

    block = (block_header*)arena;
    block->size = arena_size - BLOCK_OVERHEAD;
    block->free = 0;
    block->next = NULL;

    // Lägg in överskott i free list
    if (block->size > size) {
        split_block(block, size);
        block->next->free = 1;
        block->next->next = free_list;
        free_list = block->next;
    }

    return (void*)(block + 1);
}

void free(void* ptr) {
    if (!ptr) return;
    block_header* block = ((block_header*)ptr) - 1;
    block->free = 1;

    // Lägg tillbaka i freelist
    block->next = free_list;
    free_list = block;
}

void* realloc(void* ptr, size_t size) {
    if (!ptr) return malloc(size);
    if (size == 0) {
        free(ptr);
        return NULL;
    }

    block_header* block = ((block_header*)ptr) - 1;
    if (block->size >= size) return ptr;

    void* new_ptr = malloc(size);
    if (!new_ptr) return NULL;

    // Copy gamla data
    size_t copy_size = (block->size < size) ? block->size : size;
    for (size_t i = 0; i < copy_size; ++i)
        ((char*)new_ptr)[i] = ((char*)ptr)[i];

    free(ptr);
    return new_ptr;
}

void* calloc(size_t nmemb, size_t size) {
    size_t total = nmemb * size;
    void* ptr = malloc(total);
    if (!ptr) return NULL;

    for (size_t i = 0; i < total; ++i)
        ((char*)ptr)[i] = 0;

    return ptr;
}