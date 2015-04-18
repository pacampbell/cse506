#define __KERNEL__
#include <sys/task.h>

// static Task main_kernel_task;
// static Task *tasks;
static Task *current_task;

void initialize_tasking(void) {
}

void create_new_task(Task* task, void(*main)(), uint64_t flags, uint64_t *pml4) {
    /* Zero out the general purpose registers */
    task->registers.rax = 0;
    task->registers.rbx = 0;
    task->registers.rcx = 0;
    task->registers.rdx = 0;
    task->registers.rsi = 0;
    task->registers.rdi = 0;
    task->registers.r8 = 0;
    task->registers.r9 = 0;
    task->registers.r10 = 0;
    task->registers.r11 = 0;
    task->registers.r12 = 0;
    task->registers.r13 = 0;
    task->registers.r14 = 0;
    task->registers.r15 = 0;
    /* Set state important registers */
    task->registers.rflags = flags;
    task->registers.rip = (uint64_t)main;
    task->registers.cr3 = (uint64_t)pml4;
    task->registers.rip = (uint64_t)0; // FIXME: Need to create userspace stack; kmalloc
    /* This task is the end of the list */
    task->next = NULL;
}

void preempt(void) {
    Task *task = current_task;
    current_task = current_task->next;
    switch_tasks(&task->registers, &current_task->registers);
}

void switch_tasks(Registers *old, Registers *new) {

}
