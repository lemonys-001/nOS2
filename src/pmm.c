#include "headers.h"

#define PMM_BLOCK_SIZE 4096
#define PMM_MAX_BLOCKS 1024
#define PMM_BITMAP_SIZE (PMM_MAX_BLOCKS / 8)

static uint8_t pmm_bitmap[PMM_BITMAP_SIZE];
static uint32_t pmm_block_count = 0;
static uint32_t pmm_used_blocks = 0;

static inline void pmm_set_bit(uint32_t block) {
    pmm_bitmap[block / 8] |= (1 << (block % 8));
}

static inline void pmm_clear_bit(uint32_t block) {
    pmm_bitmap[block / 8] &= ~(1 << (block % 8));
}

static inline int pmm_test_bit(uint32_t block) {
    return (pmm_bitmap[block / 8] >> (block % 8)) & 1;
}

void pmm_init(uint32_t mem_size_kb) {
    for (int i = 0; i < PMM_BITMAP_SIZE; i++) {
        pmm_bitmap[i] = 0xFF;
    }

    pmm_block_count = (mem_size_kb * 1024) / PMM_BLOCK_SIZE;
    if (pmm_block_count > PMM_MAX_BLOCKS) {
        pmm_block_count = PMM_MAX_BLOCKS;
    }

    for (uint32_t i = 0; i < pmm_block_count; i++) {
        pmm_clear_bit(i);
    }

    pmm_used_blocks = 0;
}
uint32_t pmm_alloc_block(void) {
    for (uint32_t i = 0; i < pmm_block_count; i++) {
        if (!pmm_test_bit(i)) {
            pmm_set_bit(i);
            pmm_used_blocks++;
            return i * PMM_BLOCK_SIZE;
        }
    }
    return 0;
}

void pmm_free_block(uint32_t addr) {
    uint32_t block = addr / PMM_BLOCK_SIZE;
    if (block < pmm_block_count && pmm_test_bit(block)) {
        pmm_clear_bit(block);
        pmm_used_blocks--;
    }
}

uint32_t pmm_get_used(void) { return pmm_used_blocks; }
uint32_t pmm_get_total(void) { return pmm_block_count; }
