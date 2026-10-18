#include "headers.h"

extern void isr0(void), isr1(void), isr2(void), isr3(void), isr4(void), isr5(void), isr6(void), isr7(void);
extern void isr8(void), isr9(void), isr10(void), isr11(void), isr12(void), isr13(void), isr14(void), isr15(void);
extern void isr16(void), isr17(void), isr18(void), isr19(void), isr20(void), isr21(void), isr22(void), isr23(void);
extern void isr24(void), isr25(void), isr26(void), isr27(void), isr28(void), isr29(void), isr30(void), isr31(void);
extern void isr32(void), isr33(void), isr34(void), isr35(void), isr36(void), isr37(void), isr38(void), isr39(void);
extern void isr40(void), isr41(void), isr42(void), isr43(void), isr44(void), isr45(void), isr46(void), isr47(void);

extern void syscall_handler(void);
extern void scheduler_run(void);
extern void scheduler_tick(void);
extern void kb_handler(void);
extern void mouse_handler(void);

extern volatile int scheduler_ticks;

void *isr_table[48] = {
    (void*)&isr0,  (void*)&isr1,  (void*)&isr2,  (void*)&isr3,
    (void*)&isr4,  (void*)&isr5,  (void*)&isr6,  (void*)&isr7,
    (void*)&isr8,  (void*)&isr9,  (void*)&isr10, (void*)&isr11,
    (void*)&isr12, (void*)&isr13, (void*)&isr14, (void*)&isr15,
    (void*)&isr16, (void*)&isr17, (void*)&isr18, (void*)&isr19,
    (void*)&isr20, (void*)&isr21, (void*)&isr22, (void*)&isr23,
    (void*)&isr24, (void*)&isr25, (void*)&isr26, (void*)&isr27,
    (void*)&isr28, (void*)&isr29, (void*)&isr30, (void*)&isr31,
    (void*)&isr32, (void*)&isr33, (void*)&isr34, (void*)&isr35,
    (void*)&isr36, (void*)&isr37, (void*)&isr38, (void*)&isr39,
    (void*)&isr40, (void*)&isr41, (void*)&isr42, (void*)&isr43,
    (void*)&isr44, (void*)&isr45, (void*)&isr46, (void*)&isr47
};

void idt_install_full(void) {
    for (int i = 0; i < 48; i++) {
        idt_set_gate(i, (uint32_t)isr_table[i], 0x08, 0x8E);
    }
    idt_set_gate(128, (uint32_t)syscall_handler, 0x08, 0x8E);
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "dN"(port));
}

static inline void io_wait(void) {
    __asm__ volatile ("rep; nop" ::: "memory");
}

void isr_handler(uint32_t int_no, uint32_t err_code) {
    (void)err_code;
    if (int_no >= 32 && int_no < 48) {
        if (int_no == 32) {
            scheduler_tick();
            if (scheduler_ticks % 10 == 0) {
                scheduler_run();
            }
        } else if (int_no == 33) {
            kb_handler();
        } else if (int_no == 44) {
            mouse_handler();
        }
        if (int_no >= 40) {
            outb(0xA0, 0x20);
        }
        outb(0x20, 0x20);
        io_wait();
        return;
    }
    for (;;) {
        __asm__ volatile ("hlt");
    }
}
