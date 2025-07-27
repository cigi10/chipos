#ifndef MEMORY_H
#define MEMORY_H

#include "../include/types.h"

typedef struct {
    size_t total_memory;
    size_t used_memory;
    size_t free_memory;
    size_t num_allocations;
    size_t num_free_blocks;
} memory_stats_t;

// memory block header for allocator
typedef struct block_header {
    size_t size;
    int is_free;
    struct block_header* next;
} block_header_t;

// public API
void memory_init(void);
void* kmalloc(size_t size);
void kfree(void* ptr);
void memory_print_info(void);
memory_stats_t memory_get_stats(void);

#endif
