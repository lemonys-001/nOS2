#include "headers.h"
#ifndef NULL
#define NULL ((void *)0)
#endif

#define HEAP_MAGIC 0x12345678
#define HEAP_MIN_BLOCK_SIZE 32

struct heap_block {
    uint32_t magic;
    uint32_t size;
    int free;
    struct heap_block *next;
};

static struct heap_block *heap_start = NULL;
static uint32_t heap_size = 0;

void heap_init(void *start, uint32_t size) {
    heap_start = (struct heap_block *)start;
    heap_start->magic = HEAP_MAGIC;
    heap_start->size = size - sizeof(struct heap_block);
    heap_start->free = 1;
    heap_start->next = NULL;
    heap_size = size;
}

static void heap_split(struct heap_block *block, uint32_t size) {
    struct heap_block *new_block = (struct heap_block *)((uint8_t *)block + sizeof(struct heap_block) + size);
    new_block->magic = HEAP_MAGIC;
    new_block->size = block->size - size - sizeof(struct heap_block);
    new_block->free = 1;
    new_block->next = block->next;
    block->size = size;
    block->free = 0;
    block->next = new_block;
}

void *heap_malloc(uint32_t size) {
    if (size < HEAP_MIN_BLOCK_SIZE) {
        size = HEAP_MIN_BLOCK_SIZE;
    }

    struct heap_block *curr = heap_start;
    while (curr) {
        if (curr->free && curr->size >= size) {
            if (curr->size > size + sizeof(struct heap_block) + HEAP_MIN_BLOCK_SIZE) {
                heap_split(curr, size);
            }
            curr->free = 0;
            return (void *)((uint8_t *)curr + sizeof(struct heap_block));
        }
        curr = curr->next;
    }
    return NULL;
}

void heap_free(void *ptr) {
    if (!ptr) return;
    struct heap_block *block = (struct heap_block *)((uint8_t *)ptr - sizeof(struct heap_block));
    if (block->magic != HEAP_MAGIC) return;
    block->free = 1;

    /* Coalesce with next */
    struct heap_block *curr = heap_start;
    while (curr && curr->next) {
        if (curr->free && curr->next->free) {
            curr->size += sizeof(struct heap_block) + curr->next->size;
            curr->next = curr->next->next;
        } else {
            curr = curr->next;
        }
    }
}

void *heap_realloc(void *ptr, uint32_t size) {
    if (!ptr) return heap_malloc(size);
    if (size == 0) {
        heap_free(ptr);
        return NULL;
    }
    struct heap_block *block = (struct heap_block *)((uint8_t *)ptr - sizeof(struct heap_block));
    if (block->magic != HEAP_MAGIC) return NULL;
    if (block->size >= size) return ptr;
    void *new_ptr = heap_malloc(size);
    if (!new_ptr) return NULL;
    for (uint32_t i = 0; i < block->size && i < size; i++) {
        ((uint8_t *)new_ptr)[i] = ((uint8_t *)ptr)[i];
    }
    heap_free(ptr);
    return new_ptr;
}

uint32_t heap_get_size(void) { return heap_size; }
