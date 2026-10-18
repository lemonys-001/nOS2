#include "headers.h"

#define KB_DATA_PORT   0x60
#define KB_STATUS_PORT 0x64
#define KB_CMD_PORT    0x64

static volatile uint8_t last_scancode = 0;
static volatile int kb_has_scancode = 0;

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "dN"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "dN"(port));
}

static inline void io_wait(void) {
    __asm__ volatile ("jmp 1f; 1: jmp 2f; 2:");
}

static int kb_wait_input(void) {
    for (int i = 0; i < 100000; i++) {
        if (inb(KB_STATUS_PORT) & 1) return 1;
    }
    return 0;
}

uint8_t kb_read_scancode(void) {
    while (!kb_has_scancode) {
        __asm__ volatile ("hlt");
    }
    kb_has_scancode = 0;
    return last_scancode;
}

int kb_has_key(void) {
    return kb_has_scancode;
}

void kb_handler(void) {
    if (kb_wait_input()) {
        last_scancode = inb(KB_DATA_PORT);
        kb_has_scancode = 1;
    }
}

void kb_install(void) {
    outb(KB_CMD_PORT, 0x20);
    io_wait();
    uint8_t cc = inb(KB_DATA_PORT);
    cc &= ~0x10;
    outb(KB_CMD_PORT, 0x60);
    io_wait();
    outb(KB_DATA_PORT, cc);
    io_wait();
}
