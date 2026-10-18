#ifndef NOS_HEADERS_H
#define NOS_HEADERS_H

#include <stdint.h>

/* GDT */
void gdt_install(void);
void gdt_flush(void);

/* IDT */
void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags);
void idt_load(void);
void idt_install_null(void);
void idt_install_full(void);
void idt_load_asm(void);

/* ISR */
void isr_handler(uint32_t int_no, uint32_t err_code);

/* Keyboard */
void kb_install(void);
void kb_handler(void);
uint8_t kb_read_scancode(void);
int kb_has_key(void);

/* Mouse */
void mouse_install(void);
void mouse_handler(void);
int32_t mouse_get_x(void);
int32_t mouse_get_y(void);
uint8_t mouse_get_buttons(void);
int mouse_has_packet(void);

/* Colors */
#define C_BLACK   0
#define C_BLUE    1
#define C_WHITE   2
#define C_GRAY    3
#define C_DGRAY   4
#define C_GREEN   5
#define C_YELLOW  6
#define C_RED     7

/* Graphics */
void gfx_init(void);
void gfx_clear(uint8_t color);
void gfx_pixel(int x, int y, uint8_t color);
void gfx_rect(int x, int y, int w, int h, uint8_t color);
void gfx_line(int x0, int y0, int x1, int y1, uint8_t color);
void gfx_flush(void);
void draw_char(int x, int y, char c, uint8_t fg, uint8_t bg);
void draw_string(int x, int y, const char *s, uint8_t fg, uint8_t bg);

/* Timer */
void timer_init(uint32_t freq);

/* Scheduler */
int task_create(void (*entry)(void), uint32_t priority);
int task_get_current(void);
void scheduler_run(void);
extern struct task {
    uint32_t esp;
    uint32_t stack[1024];
    int used;
    int state;
} tasks[];
extern int task_count;
extern volatile int scheduler_ticks;

#endif
