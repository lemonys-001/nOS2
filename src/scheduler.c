#include "headers.h"

#define MAX_TASKS 8
#define STACK_SIZE 4096

struct task tasks[MAX_TASKS];
int task_count = 0;
int current_task = 0;
volatile int scheduler_ticks = 0;

extern void schedule_switch(uint32_t *old_esp, uint32_t new_esp);

int task_create(void (*entry)(void), uint32_t priority) {
    (void)priority;
    if (task_count >= MAX_TASKS) return -1;

    int idx = -1;
    for (int i = 0; i < MAX_TASKS; i++) {
        if (!tasks[i].used) { idx = i; break; }
    }
    if (idx == -1) return -1;

    struct task *t = &tasks[idx];
    uint32_t *stack = &t->stack[STACK_SIZE / 4 - 1];

    /* Build initial stack frame as if iret just happened */
    *stack-- = 0x00000002;          /* EFLAGS (IF set) */
    *stack-- = 0x08;                /* CS */
    *stack-- = (uint32_t)entry;     /* EIP */
    *stack-- = 0x00;                /* error code (dummy) */
    *stack-- = 0x10;                /* DS */
    *stack-- = 0x10;                /* ES */
    *stack-- = 0x10;                /* FS */
    *stack-- = 0x10;                /* GS */
    *stack-- = 0x10;                /* SS */
    *stack-- = 0x00;                /* EDI */
    *stack-- = 0x00;                /* ESI */
    *stack-- = 0x00;                /* EBP */
    *stack-- = 0x00;                /* ESP (dummy) */
    *stack-- = 0x00;                /* EBX */
    *stack-- = 0x00;                /* EDX */
    *stack-- = 0x00;                /* ECX */
    *stack-- = 0x00;                /* EAX */

    t->esp = (uint32_t)(stack + 1);
    t->used = 1;
    t->state = 1; /* ready */

    task_count++;
    return idx;
}

void task_yield(void) {
    __asm__ volatile ("int $0x81");
}

int task_get_current(void) {
    return current_task;
}

void scheduler_tick(void) {
    scheduler_ticks++;
}

void scheduler_run(void) {
    if (task_count == 0) return;

    int next = (current_task + 1) % MAX_TASKS;
    int attempts = 0;
    while (attempts < MAX_TASKS) {
        if (tasks[next].used && tasks[next].state == 1) {
            break;
        }
        next = (next + 1) % MAX_TASKS;
        attempts++;
    }

    if (attempts >= MAX_TASKS) return;

    struct task *curr = &tasks[current_task];
    struct task *nxt = &tasks[next];

    if (curr->state == 1) {
        curr->state = 1;
    }

    current_task = next;
    nxt->state = 0;

    schedule_switch(&curr->esp, nxt->esp);
}
