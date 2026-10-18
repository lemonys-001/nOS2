section .text
global idt_load_asm
idt_load_asm:
    lidt [esp + 4]
    sti
    ret
