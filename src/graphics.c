#include "headers.h"

#define VGA_WIDTH   320
#define VGA_HEIGHT  200
#define VGA_ADDR    0xA0000
#define VGA_GC_INDEX 0x3CE
#define VGA_GC_MAP_MASK 8

static volatile uint8_t *vga_fb = (uint8_t *)VGA_ADDR;
static uint8_t backbuffer[VGA_WIDTH * VGA_HEIGHT];

/* Windows 7 Blue #00A2ED */
static const uint8_t win7_blue_r = 0;
static const uint8_t win7_blue_g = 162;
static const uint8_t win7_blue_b = 237;

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "dN"(port));
}
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "dN"(port));
    return ret;
}
static inline void io_wait(void) {
    __asm__ volatile ("rep; nop" ::: "memory");
}

void gfx_set_palette_color(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    outb(0x3C8, index);
    io_wait();
    outb(0x3C9, r >> 2);
    io_wait();
    outb(0x3C9, g >> 2);
    io_wait();
    outb(0x3C9, b >> 2);
    io_wait();
}

void gfx_init(void) {
    gfx_set_palette_color(0, 0, 0, 0);        /* black */
    gfx_set_palette_color(1, win7_blue_r, win7_blue_g, win7_blue_b); /* Windows 7 Blue */
    gfx_set_palette_color(2, 255, 255, 255); /* white */
    gfx_set_palette_color(3, 192, 192, 192); /* light gray */
    gfx_set_palette_color(4, 128, 128, 128); /* dark gray */
    gfx_set_palette_color(5, 0, 128, 0);     /* green */
    gfx_set_palette_color(6, 255, 255, 0);   /* yellow */
    gfx_set_palette_color(7, 255, 0, 0);     /* red */
}

void gfx_clear(uint8_t color) {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        backbuffer[i] = color;
    }
}

void gfx_pixel(int x, int y, uint8_t color) {
    if (x < 0 || x >= VGA_WIDTH || y < 0 || y >= VGA_HEIGHT) return;
    backbuffer[y * VGA_WIDTH + x] = color;
}

void gfx_rect(int x, int y, int w, int h, uint8_t color) {
    for (int dy = 0; dy < h; dy++) {
        for (int dx = 0; dx < w; dx++) {
            gfx_pixel(x + dx, y + dy, color);
        }
    }
}

void gfx_line(int x0, int y0, int x1, int y1, uint8_t color) {
    int dx = x1 - x0; if (dx < 0) dx = -dx;
    int dy = y1 - y0; if (dy < 0) dy = -dy;
    int sx = x0 < x1 ? 1 : -1;
    int sy = y0 < y1 ? 1 : -1;
    int err = dx - dy;
    while (1) {
        gfx_pixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx) { err += dx; y0 += sy; }
    }
}

void gfx_flush(void) {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_fb[i] = backbuffer[i];
    }
}
