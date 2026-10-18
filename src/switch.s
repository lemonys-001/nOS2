section .text
global schedule_switch
global syscall_handler

; schedule_switch(uint32_t *old_esp, uint32_t new_esp)
; Called from C. Args on stack: [esp+4]=old_esp_ptr, [esp+8]=new_esp
schedule_switch:
    pushad
    pushf
    mov eax, [esp + 4 + 36]    ; old_esp_ptr (after pushad(32)+pushf(4)+retaddr(4))
    mov [eax], esp             ; save current esp

    mov eax, [esp + 4 + 36 + 4] ; new_esp
    mov esp, eax               ; switch to new stack

    popf
    popad
    ret

; syscall_handler - called via int 0x80
; eax = syscall number, ebx/ecx/edx = args
syscall_handler:
    pushad
    pushf
    call syscall_dispatch
    popf
    popad
    iretd

extern syscall_dispatch
