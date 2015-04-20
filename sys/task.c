#define __KERNEL__
#include <sys/task.h>

extern void printk();

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
            printk("name: %s pid: %d\n", task->name == NULL ? "NULL" : task->name, task->pid);
            printk("state: [%s](%d) priority: 0x%x\n", state_map[task->state], task->state, task->priority);
            printk("previous: %p next: %p\n", task->prev, task->next);
            printk("stack: %p\n", task->stack);
            /* Dump register contents */
            printk("[Register Dump]\n");
            printk("rax: 0x%x rbx: 0x%x rcx: 0x%x rdx: 0x%x\n", task->registers.rax, task->registers.rbx, task->registers.rcx, task->registers.rdx);
            printk(" r8: 0x%x  r9: 0x%x r10: 0x%x r11: 0x%x\n", task->registers.r8, task->registers.r9, task->registers.r10, task->registers.r11);
            printk("r12: 0x%x r13: 0x%x r14: 0x%x r15: 0x%x\n", task->registers.r12, task->registers.r13, task->registers.r14, task->registers.r15);
            printk("rsi: 0x%x rdi: 0x%x\n", task->registers.rsi, task->registers.rdi);
            printk("rbp: 0x%x rip: 0x%x rsp: 0x%x\n", task->registers.rbp, task->registers.rip, task->registers.rsp);
            printk("cr3: 0x%x rflags: 0x%x\n", task->registers.cr3, task->registers.rflags);
        }
    #endif
}

void initialize_scheudler(void(*kmain)()) {
    // Allocate a new task
    Task *kernel_task = (Task*) PHYS_TO_VIRT(kmalloc_pg());
    memset(kernel_task, 0, sizeof(Task));
    // Get the kernel pml4
    pml4_t *kernel_pml4 = (pml4_t *)get_cr3();
    // Get the kernel flags
    uint64_t kernel_rflags = get_rflags();
    // Create new task
    create_new_task(kernel_task, kmain, "KERNEL", MAX_PRIORITY, kernel_rflags, (uint64_t*)kernel_pml4);
    // Dump the task
    dump_task(kernel_task);
    // Assign the kernel as the current task
    current_task = kernel_task;
    // Add the kernel to the list of tasks
    insert_into_list(&tasks, kernel_task);
}

void initialize_task(const char *name, priority_t priority, void(*main)()) {
    // Allocate a new task
    Task *task = (Task*) PHYS_TO_VIRT(kmalloc_pg());
    memset(task, 0, sizeof(Task));
    // Allocate a new pml4
    pml4_t *task_pml4 = (pml4_t *) PHYS_TO_VIRT(kmalloc_pg());
    memset(task_pml4, 0, PAGE_SIZE);        // Zero out memory
    task_pml4 = (pml4_t *) VIRT_TO_PHYS(task_pml4); // Convert address back to physical for cr3
    // Grab the current state of rflags
    uint64_t rflags = get_rflags();
    // Create new task
    create_new_task(task, main, name, priority, rflags, (uint64_t*)task_pml4);
    // Insert this task into the list
    insert_into_list(&tasks, task);
}

void create_new_task(Task* task, void(*main)(), const char *name, priority_t priority, uint64_t flags, uint64_t *pml4) {
    task->name = name;
    task->priority = priority;
    task->state = NEW;
    task->pid = allocate_pid();
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
    task->registers.rsp = (uint64_t)task->stack;
    /* This task is the end of the list */
    task->next = NULL;
    task->prev = 0;
}

void preempt(void) {
    Task *task = current_task;
    current_task = current_task->next;
    if(current_task == NULL) {
        // Set the current task back to the head of the list
        current_task = tasks;
    }
    printk("REMOVE ME %p\n", task);
    // Attempt to switch tasks; Assembly magic voodo
    // switch_tasks(task, current_task);
}

void switch_tasks(Task *old, Task *new) {
    printk("%d\n", sizeof(Task));
    // dump_task(old);
    // Make sure both are not null
    // and both are not the same (no need to swap if same)
    if(old != NULL && new != NULL /* && old != new */) {
        // Get the cr3 value
        old->registers.cr3 = (uint64_t) get_cr3();
        // Get the flags value
        old->registers.rflags = get_rflags();
        __asm__ __volatile__(
            /* Push all registers onto stack */
            "pushfq;"
            /* Save register state for current process */
            "movq %%rax, 0(%0);"
            "movq %%rbx, 8(%0);"
            "movq %%rcx, 16(%0);"
            "movq %%rdx, 24(%0);"
            "movq %%rsi, 32(%0);"
            "movq %%rdi, 40(%0);"
            "movq %%rbp, 48(%0);"
            "movq $., 56(%0);"
            "movq %%rsp, 64(%0);"
            "movq %%r8, 72(%0);"
            "movq %%r9, 80(%0);"
            "movq %%r10, 88(%0);"
            "movq %%r11, 96(%0);"
            "movq %%r12, 104(%0);"
            "movq %%r13, 112(%0);"
            "movq %%r14, 120(%0);"
            "movq %%r15, 128(%0);"
            // "cli; hlt;"
            /* Put the new values in */
            "movq 0(%1), %%rax;"
            "movq 8(%1), %%rbx;"
            "movq 16(%1), %%rcx;"
            "movq 24(%1), %%rdx;"
            "movq 32(%1), %%rsi;"
            "movq 40(%1), %%rdi;"
            "movq 48(%1), %%rbp;"
            "movq 64(%1), %%rsp;"
            "movq 72(%1), %%r8;"
            "movq 80(%1), %%r9;"
            "movq 88(%1), %%r10;"
            "movq 96(%1), %%r11;"
            "movq 104(%1), %%r12;"
            "movq 112(%1), %%r13;"
            "movq 120(%1), %%r14;"
            "movq 128(%1), %%r15;"
            /* TODO: START HERE */

            // "movq 0(%1), %%rax;"
            // "movq ;"
            : "=r"(old)
            : "r"(new)
            :
        );
    }
    printk("\n");
    dump_task(old);
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
