#include "memory.h"
#include "../drivers/console.h"
#include "../include/kernel.h"
#include "../../lib/string.h"

// heap allocator constants
#define HEAP_SIZE (1024 * 1024)  // 1MB heap
#define MIN_BLOCK_SIZE 32
#define BLOCK_HEADER_SIZE sizeof(block_header_t)

// declarations for static functions
static block_header_t* find_free_block(size_t size);
static void split_block(block_header_t* block, size_t size);
static void merge_free_blocks(void);

// global heap memory (static allocation for simplicity)
static char heap_memory[HEAP_SIZE];
static block_header_t* heap_start = NULL;
static int memory_initialized = 0;

// statistics
static memory_stats_t stats = {0};

void memory_init(void) {
    if (memory_initialized) {
        return;
    }
    
    // Initialize the first block to cover the entire heap
    heap_start = (block_header_t*)heap_memory;
    heap_start->size = HEAP_SIZE - BLOCK_HEADER_SIZE;
    heap_start->is_free = 1;
    heap_start->next = NULL;
    
    // Initialize statistics
    stats.total_memory = HEAP_SIZE;
    stats.used_memory = BLOCK_HEADER_SIZE; // Just the header initially
    stats.free_memory = HEAP_SIZE - BLOCK_HEADER_SIZE;
    stats.num_allocations = 0;
    stats.num_free_blocks = 1;
    
    memory_initialized = 1;
    
    console_puts("Memory manager initialized with ");
    console_put_hex(HEAP_SIZE);
    console_puts(" bytes\n");
}

void* kmalloc(size_t size) {
    if (!memory_initialized) {
        kernel_panic("kmalloc called before memory_init");
        return NULL;
    }
    
    if (size == 0) {
        return NULL;
    }
    
    // align size to 8-byte boundary
    size = (size + 7) & ~7;
    
    // ensure minimum block size
    if (size < MIN_BLOCK_SIZE) {
        size = MIN_BLOCK_SIZE;
    }
    
    block_header_t* block = find_free_block(size);
    
    if (!block) {
        return NULL;
    }
    
    // split the block if it's much larger than needed
    if (block->size > size + BLOCK_HEADER_SIZE + MIN_BLOCK_SIZE) {
        split_block(block, size);
    }
    
    // mark block as used
    block->is_free = 0;
    
    // update statistics
    stats.used_memory += block->size + BLOCK_HEADER_SIZE;
    stats.free_memory -= block->size + BLOCK_HEADER_SIZE;
    stats.num_allocations++;
    
    // return pointer to data (after header)
    return (char*)block + BLOCK_HEADER_SIZE;
}

void kfree(void* ptr) {
    if (!ptr || !memory_initialized) {
        return;
    }
    
    // get the block header
    block_header_t* block = (block_header_t*)((char*)ptr - BLOCK_HEADER_SIZE);
    
    // validate the block
    if ((char*)block < heap_memory || 
        (char*)block >= heap_memory + HEAP_SIZE ||
        block->is_free) {
        console_println("WARNING: Invalid free() call");
        return;
    }
    
    // mark block as free
    block->is_free = 1;
    
    // update statistics
    stats.used_memory -= block->size + BLOCK_HEADER_SIZE;
    stats.free_memory += block->size + BLOCK_HEADER_SIZE;
    stats.num_allocations--;
    
    // merge adjacent free blocks
    merge_free_blocks();
}

void memory_print_info(void) {
    console_println("\n--- Memory Information ---");
    console_puts("Total memory: ");
    console_put_hex(stats.total_memory);
    console_puts(" bytes\n");
    
    console_puts("Used memory: ");
    console_put_hex(stats.used_memory);
    console_puts(" bytes\n");
    
    console_puts("Free memory: ");
    console_put_hex(stats.free_memory);
    console_puts(" bytes\n");
    
    console_puts("Active allocations: ");
    console_put_hex(stats.num_allocations);
    console_puts("\n");
    
    console_puts("Free blocks: ");
    console_put_hex(stats.num_free_blocks);
    console_puts("\n");
    
    // show fragmentation info
    console_puts("Heap utilization: ");
    if (stats.total_memory > 0) {
        uint32_t utilization = (stats.used_memory * 100) / stats.total_memory;
        console_put_hex(utilization);
        console_puts("%\n");
    } else {
        console_puts("0%\n");
    }
}

memory_stats_t memory_get_stats(void) {
    // update free block count
    stats.num_free_blocks = 0;
    block_header_t* current = heap_start;
    
    while (current) {
        if (current->is_free) {
            stats.num_free_blocks++;
        }
        current = current->next;
    }
    
    return stats;
}

// internal helper functions
static block_header_t* find_free_block(size_t size) {
    block_header_t* current = heap_start;
    
    while (current) {
        if (current->is_free && current->size >= size) {
            return current;
        }
        current = current->next;
    }
    
    return NULL;
}

static void split_block(block_header_t* block, size_t size) {
    if (block->size <= size + BLOCK_HEADER_SIZE + MIN_BLOCK_SIZE) {
        // block is too small to split
        return;
    }
    
    // create new block for the remaining space
    block_header_t* new_block = (block_header_t*)((char*)block + BLOCK_HEADER_SIZE + size);
    new_block->size = block->size - size - BLOCK_HEADER_SIZE;
    new_block->is_free = 1;
    new_block->next = block->next;
    
    // update current block
    block->size = size;
    block->next = new_block;
}

static void merge_free_blocks(void) {
    block_header_t* current = heap_start;
    
    while (current && current->next) {
        if (current->is_free && current->next->is_free) {
            // check if blocks are adjacent
            char* current_end = (char*)current + BLOCK_HEADER_SIZE + current->size;
            char* next_start = (char*)current->next;
            
            if (current_end == next_start) {
                // merge the blocks
                current->size += BLOCK_HEADER_SIZE + current->next->size;
                current->next = current->next->next;
                continue; // don't advance current, check for more merges
            }
        }
        current = current->next;
    }
}
