#include "headers.h"

#define KB_DATA_PORT   0x60
#define KB_STATUS_PORT 0x64
#define KB_CMD_PORT    0x64

static volatile int32_t mouse_x = 0;
static volatile int32_t mouse_y = 0;
static volatile uint8_t mouse_buttons = 0;
static volatile int mouse_packet_ready = 0;
static volatile int mouse_byte = 0;
static volatile uint8_t mouse_buf[3];

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

static int kb_wait_output(void) {
    for (int i = 0; i < 100000; i++) {
        if (!(inb(KB_STATUS_PORT) & 2)) return 1;
    }
    return 0;
}

void mouse_write(uint8_t val) {
    kb_wait_output();
    outb(KB_CMD_PORT, 0xD4);
    io_wait();
    kb_wait_output();
    outb(KB_DATA_PORT, val);
}

uint8_t mouse_read(void) {
    kb_wait_input();
    return inb(KB_DATA_PORT);
}

void mouse_handler(void) {
    uint8_t data = inb(KB_DATA_PORT);
    mouse_buf[mouse_byte++] = data;

    if (mouse_byte == 3) {
        mouse_byte = 0;
        uint8_t buttons = mouse_buf[0] & 0x07;
        int8_t x = (int8_t)mouse_buf[1];
        int8_t y = (int8_t)mouse_buf[2];

        if (mouse_buf[0] & 0x10) x |= 0xFFFFFF00;
        if (mouse_buf[0] & 0x20) y |= 0xFFFFFF00;

        mouse_x += x;
        mouse_y -= y;  /* Y is inverted on screen */
        if (mouse_x < 0) mouse_x = 0;
        if (mouse_y < 0) mouse_y = 0;
        mouse_buttons = buttons;
        mouse_packet_ready = 1;
    }
}

int32_t mouse_get_x(void) { return mouse_x; }
int32_t mouse_get_y(void) { return mouse_y; }
uint8_t mouse_get_buttons(void) { return mouse_buttons; }
int mouse_has_packet(void) { return mouse_packet_ready; }

void mouse_install(void) {
    kb_wait_output();
    outb(KB_CMD_PORT, 0xA8);
    io_wait();

    kb_wait_output();
    outb(KB_CMD_PORT, 0x20);
    io_wait();
    uint8_t cc = inb(KB_DATA_PORT);
    cc |= 0x02;
    outb(KB_CMD_PORT, 0x60);
    io_wait();
    outb(KB_DATA_PORT, cc);
    io_wait();

    mouse_write(0xF6);  /* set defaults */
    mouse_read();
    mouse_write(0xF4);  /* enable streaming */
    mouse_read();
}
