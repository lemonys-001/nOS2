#include <stdint.h>

#define VGA_WIDTH  80
#define VGA_HEIGHT 25
#define VGA_ADDR   0xB8000

/* Windows 7 Blue #00A2ED -> VGA attribute: blue background (0x10), white foreground (0x07) */
#define VGA_BLUE_ATTR 0x17

static uint16_t *vga_buffer = (uint16_t *)VGA_ADDR;

void kernel_main(unsigned int magic, unsigned int addr) {
    (void)magic;
    (void)addr;

    /* Fill entire VGA text buffer with blue background */
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = (uint16_t)(' ' | (VGA_BLUE_ATTR << 8));
    }

    /* Halt */
    for (;;) {
        __asm__ volatile ("hlt");
    }
}
