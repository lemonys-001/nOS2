#include "headers.h"

extern void scheduler_run(void);
extern int task_get_current(void);

#define SYSCALL_PRINT_CHAR 1
#define SYSCALL_PRINT_STR  2
#define SYSCALL_EXIT       3
#define SYSCALL_YIELD      4
#define SYSCALL_GET_PID    5

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "dN"(port));
}
static inline void io_wait(void) {
    __asm__ volatile ("rep; nop" ::: "memory");
}

static int sys_console_x = 0;
static int sys_console_y = 150;

static void sys_putc(char c) {
    if (c == '\n') {
        sys_console_x = 0;
        sys_console_y += 8;
        if (sys_console_y >= 192) sys_console_y = 150;
        return;
    }
    draw_char(sys_console_x, sys_console_y, c, C_WHITE, C_BLUE);
    sys_console_x += 6;
    if (sys_console_x >= 300) { sys_console_x = 0; sys_console_y += 8; }
}

int syscall_dispatch(uint32_t eax, uint32_t ebx, uint32_t ecx, uint32_t edx) {
    (void)ecx; (void)edx;
    switch (eax) {
        case SYSCALL_PRINT_CHAR:
            sys_putc((char)ebx);
            break;
        case SYSCALL_PRINT_STR:
            {
                const char *s = (const char *)ebx;
                while (*s) sys_putc(*s++);
            }
            break;
        case SYSCALL_EXIT:
            tasks[task_get_current()].used = 0;
            task_count--;
            scheduler_run();
            break;
        case SYSCALL_YIELD:
            scheduler_run();
            break;
        case SYSCALL_GET_PID:
            return task_get_current();
        default:
            break;
    }
    return 0;
}

void timer_init(uint32_t frequency) {
    uint32_t divisor = 1193180 / frequency;
    uint8_t low = (uint8_t)(divisor & 0xFF);
    uint8_t high = (uint8_t)((divisor >> 8) & 0xFF);

    outb(0x43, 0x36);
    io_wait();
    outb(0x40, low);
    io_wait();
    outb(0x40, high);
    io_wait();
}
