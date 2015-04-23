#define __KERNEL__
#include <sys/task.h>

extern struct tss_t tss;

static Task *tasks;
static Task *current_task;

/* Used for assigning a pid to a task */
static uint64_t pid_map[PID_MAP_LENGTH];

/**
 * Finds the first unused pid in the pid_map.
 * @return Returns an unused PID if one exists, else -1.
 */
static pid_t allocate_pid(void) {
    // Scan pid map for not in use
    pid_t new_pid = -1;
    for(size_t i = 0; i < PID_MAP_LENGTH; i++) {
        // Check to see if there is a free bit to mark
        if(pid_map[i] != 0xffffffffffffffff) {
            // Find the first free bit; 64 is the magic number for a word size in bits
            for(size_t j = 0; j < 64; j++) {
                bool free_bit = !(pid_map[i] >> j & 1);
                if(free_bit) {
                    // Set the bit in the map
                    uint64_t mask = 1 << j;
                    pid_map[i] |= mask;
                    // Generate the pid
                    new_pid = (i * 64) + j;
                    // We found a pid, lets leave
                    goto done;
                }
            }
        }
    }
done:
    return new_pid;
}

/**
 * Marks a pid as no longer in use in the pid_map.
 * @param pid Entry to mark as free.
 */
static void free_pid(pid_t pid) {
    // Make sure we didn't get a bad pid
    if(pid >= 0){
        // Convert the pid into a position in the pid map
        size_t array_index = pid / 64;
        size_t bit_mask = ~(1 << (pid % 64));
        pid_map[array_index] &= bit_mask;
    }
}

/**
 * Gets the current value of rflags.
 * @return Returns the rflags value.
 */
static uint64_t get_rflags(void) {
    uint64_t rflags;
    __asm__ __volatile__(
        "pushfq;"
        "popq %0;"
        : "=r"(rflags)
        : /* input values */
        : /* clobbered regs */
        );
    return rflags;
}

/**
 * Prints out the values of a task
 * @param task Task to display values of.
 */
static void dump_task(Task *task) {
    #ifdef DEBUG
        if(task != NULL) {
            static char *state_map[5] = {"NEW", "READY", "RUNNING", "WAITING", "TERMINATED"};
            /* Dump kernel meta data */
            printk("[Task State Dump]\n");
            printk("Address: %p\n", task);
            printk("name: %s pid: %d type: %s\n", task->name == NULL ? "NULL" : task->name, task->pid, task->type == KERNEL ? "KERNEL" : "USER");
            printk("state: [%s](%d) priority: 0x%x\n", state_map[task->state], task->state, task->priority);
            printk("previous: %p next: %p\n", task->prev, task->next);
            printk("stack: %p\n", task->stack);
            // /* Dump register contents */
            printk("[Register Dump]\n");
            printk("rax: 0x%x rbx: 0x%x rcx: 0x%x rdx: 0x%x\n", task->registers.rax, task->registers.rbx, task->registers.rcx, task->registers.rdx);
            printk(" r8: 0x%x  r9: 0x%x r10: 0x%x r11: 0x%x\n", task->registers.r8, task->registers.r9, task->registers.r10, task->registers.r11);
            printk("r12: 0x%x r13: 0x%x r14: 0x%x r15: 0x%x\n", task->registers.r12, task->registers.r13, task->registers.r14, task->registers.r15);
            printk("rsi: 0x%x rdi: 0x%x\n", task->registers.rsi, task->registers.rdi);
            printk("rip: %p rbp: %p rsp: %p\n", task->registers.rip, task->registers.rbp, task->registers.rsp);
            printk("cr3: %p rflags: 0x%x\n\n", task->registers.cr3, task->registers.rflags);
        }
    #endif
}

Task* create_kernel_task(const char *name, void(*code)()) {
    // Get the kernel page tables
    pml4_t *kernel_pml4 = (pml4_t *)get_cr3();
    // Get the kernel flags
    uint64_t kernel_rflags = get_rflags();
    // Allocate space for a new kernel task
    Task *kernel_task = (Task*) PHYS_TO_VIRT(kmalloc_pg());
    // Create space for a new stack
    uint64_t kernel_task_stack = (uint64_t) PHYS_TO_VIRT(kmalloc_pg());
    // Initialize the task
    create_new_task(kernel_task, name, KERNEL, MAX_PRIORITY, kernel_rflags,
                    kernel_pml4, kernel_task_stack, code);
    // Add the task to the scheduler list
    insert_into_list(&tasks, kernel_task);
    // Print out contents of the task
    dump_task(kernel_task);
    return kernel_task;
}

void setup_stack(Task *task) {
    __asm__ __volatile__(
        /* Store the initial stack pointer */
        "movq %%rsp, %%rax;"
        "movq %1, %%rcx;"
        /* Set the stack pointer to the new tasks stack */
        "movq %2, %%rsp;"
        /* Push the instruction pointer onto the stack */
        "pushq %3;"
        /* Push all of the registers onto the stack */
        "pushq %4;" // rbp
        "pushq %5;" // rax
        "pushq %6;" // rbx
        "pushq %7;" // rcx
        "pushq %8;" // rdx
        "pushq %9;" // rsi
        "pushq %10;" // rdi
        "pushq %11;" // r8
        "pushq %12;" // r9
        "pushq %13;" // r10
        "pushq %14;" // r11
        "pushq %15;" // r12
        "pushq %16;" // r13
        "pushq %17;" // r14
        "pushq %18;" // r15
        /* Save the new value of the stack pointer */
        "movq %%rsp, 64(%%rcx);"
        /* Set the stack pointer back to what it was */
        "movq %%rax, %%rsp;"
        : "=r"(task)
        : "m"(task), "m"(task->registers.rsp), "m"(task->registers.rip), "m"(task->registers.rbp), "m"(task->registers.rax), "m"(task->registers.rbx), "m"(task->registers.rcx), "m"(task->registers.rdx), "m"(task->registers.rsi), "m"(task->registers.rdi), "m"(task->registers.r8), "m"(task->registers.r9), "m"(task->registers.r10), "m"(task->registers.r11), "m"(task->registers.r12), "m"(task->registers.r13), "m"(task->registers.r14), "m"(task->registers.r15)
        : "rax", "rcx", "rdi"
    );
}

Task* create_new_task(Task* task, const char *name, task_type_t type, priority_t priority, uint64_t flags, pml4_t *pml4, uint64_t stack, void(*main)()) {
    task->name = name;
    task->priority = priority;
    task->state = NEW;
    task->type = type;
    task->pid = allocate_pid();
    /* Set the address of the stack */
    task->stack = stack;
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
    task->registers.rsp = stack + PAGE_SIZE - 8; // Start the stack pointer at the other side
    task->registers.rbp = task->registers.rsp;
    /* This task is the end of the list */
    task->next = NULL;
    task->prev = NULL;
    // Initialize the stack
    setup_stack(task);
    return task;
}

void preempt(void) {
    Task *task = current_task;
    current_task = current_task->next;
    if(current_task == NULL) {
        // Set the current task back to the head of the list
        current_task = tasks;
    }
    // Attempt to switch tasks; Assembly magic voodo
    switch_tasks(task, current_task);
}

void set_task(Task *task) {
    current_task = task;
    task->state = RUNNING;
    // Set cr3
    // set_cr3((pml4_t*)(task->registers.cr3));
    // Set the control register
    __asm__ __volatile__(
        "movq %0, %%rsp;"
        "popq %%r15;"
        "popq %%r14;"
        "popq %%r13;"
        "popq %%r12;"
        "popq %%r11;"
        "popq %%r10;"
        "popq %%r9;"
        "popq %%r8;"
        "popq %%rdi;"
        "popq %%rsi;"
        "popq %%rdx;"
        "popq %%rcx;"
        "popq %%rbx;"
        "popq %%rax;"
        "popq %%rbp;"
        :
        : "m"(task->registers.rsp)
        :
    );
}

void switch_tasks(Task *old, Task *new) {
    printk("old: %p new: %p\n", old, new);
    // Make sure both are not null
    // and both are not the same (no need to swap if same)
    if(old != NULL && new != NULL && old != new) {
        old->state = READY;
        new->state = RUNNING;
        // Get the cr3 value
        old->registers.cr3 = (uint64_t) get_cr3();
        // Get the flags value
        old->registers.rflags = get_rflags();
        /* Save the current register state */
        __asm__ __volatile__(
            /* Save registers */
             "movq %%rax, %3;"
             "movq %%rbx, %4;"
             "movq %%rcx, %5;"
             "movq %%rdx, %6;"
             "movq %%rsi, %7;"
             "movq %%rdi, %8;"
             "movq %%r8, %9;"
             "movq %%r9, %10;"
             "movq %%r10, %11;"
             "movq %%r11, %12;"
             "movq %%r12, %13;"
             "movq %%r13, %14;"
             "movq %%r14, %15;"
             "movq %%r15, %16;"
             /* Save special registers */
             "movq %%rsp, %0;"
             "movq %%rbp, %2;"
             /* Get the instruction pointer*/
             "popq %%rax;"
             "popq %%rax;"
             : "=m"(old->registers.rsp), "=m"(old->registers.rip), "=m"(old->registers.rbp), "=m"(old->registers.rax), "=m"(old->registers.rbx), "=m"(old->registers.rcx), "=m"(old->registers.rdx), "=m"(old->registers.rsi), "=m"(old->registers.rdi), "=m"(old->registers.r8), "=m"(old->registers.r9), "=m"(old->registers.r10), "=m"(old->registers.r11), "=m"(old->registers.r12), "=m"(old->registers.r13), "=m"(old->registers.r14), "=m"(old->registers.r15)
             : 
             : "rax", "memory"
        );
        /* Prepare the stack for the next time its called */
        setup_stack(old);
        // printk("rip: %p\n", old->registers.rip);
        //  __asm__ __volatile__("cli; hlt;");
        dump_task(old);
        // Now set the new task
        set_task(new);
    }
    // SHOULD NEVER GET HERE ?
    printk("If you see this something probably went wrong.\n");
}

bool insert_into_list(Task **list, Task *task) {
    bool success = false;
    if(list != NULL && task != NULL) {
        if(*list == NULL) {
            // First task to be added to the list
            *list = task;
            task->prev = NULL;
            task->next = NULL;
        } else {
            // Just set the old head to the next field of the new list
            task->next = *list;
            task->prev = NULL;      // This new task is now the head so its previous is NULL.
            (*list)->prev = task;     // Set the old heads previous to the new head
            // Set the head of the list to the new task
            *list = task;
            // Mark the operation as a success
        }
        success = true;
    }
    return success;
}

Task *get_task_by_pid(pid_t pid) {
    return NULL;
}

Task *remove_task_by_pid(pid_t pid) {

    free_pid(pid);  // Give back the pid so it can be reassigned.

    return NULL;
}
