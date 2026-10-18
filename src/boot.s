section .multiboot
align 4
    dd 0x1BADB002
    dd 0x00000003
    dd -(0x1BADB002 + 0x00000003)

section .bss
align 4
stack_bottom:
    resb 16384
stack_top:

section .text
global _start
extern kernel_main
_start:
    ; Set VGA mode 13h (320x200, 256 colors)
    mov ax, 0x0013
    int 0x10

    ; Set up stack
    mov esp, stack_top
    call kernel_main
.hang:
    cli
    hlt
    jmp .hang
