#define __KERNEL__
#include <sys/task.h>
#include <sys/elf.h>

static Task *tasks;
static Task *current_task;
static int task_count = 0;

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
    if(pid >= 0) {
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
void dump_task(Task *task) {
    #ifdef DEBUG
        if(task != NULL) {
            static char *state_map[5] = {"NEW", "READY", "RUNNING", "WAITING", "TERMINATED"};
            /* Dump kernel meta data */
            printk("[Task State Dump - %s - pid: %d] Address: %p\n", 
                task->name == NULL ? "NULL" : task->name, task->pid, task);
            printk("type: %s state: [%s](%d) priority: 0x%x\n",  
                task->type == KERNEL ? "KERNEL" : "USER",
                state_map[task->state],
                task->state,
                task->priority
            );
            printk("stack: %p previous: %p next: %p\n", task->stack, task->prev, task->next);
            // /* Dump register contents */
            printk("[Register Dump - %s - pid: %d]\n", task->name == NULL ? "NULL" : task->name, task->pid, task);
            printk("rax: 0x%x rbx: 0x%x rcx: 0x%x rdx: 0x%x\n", task->registers.rax, task->registers.rbx, task->registers.rcx, task->registers.rdx);
            printk(" r8: 0x%x  r9: 0x%x r10: 0x%x r11: 0x%x\n", task->registers.r8, task->registers.r9, task->registers.r10, task->registers.r11);
            printk("r12: 0x%x r13: 0x%x r14: 0x%x r15: 0x%x\n", task->registers.r12, task->registers.r13, task->registers.r14, task->registers.r15);
            printk("rsi: 0x%x rdi: 0x%x  cr3: %p rflags: 0x%x\n", 
                task->registers.rsi, task->registers.rdi, task->registers.cr3, 
                task->registers.rflags);
            printk("rip: %p rbp: %p rsp: %p\n\n",
                task->registers.rip, task->registers.rbp, task->registers.rsp);
        } else {
            panic("=== NULL TASK STRUCT - UNABLE TO DUMP ===\n");
        }
    #endif
}
        
Task* create_user_elf_task(const char *name, char* elf, uint64_t size) {
    // Get the kernel page tables
    pml4_t *kernel_pml4 = (pml4_t *)get_cr3();
    // Copy the kernels page tables
    pml4_t *user_pml4 = copy_page_tables(kernel_pml4);
    // Get the kernel flags
    uint64_t kernel_rflags = get_rflags();
    // Allocate space for a new user task
    Task *user_task = (Task*) PHYS_TO_VIRT(kmalloc_pg());
    //user_task->mm = load_elf(elf, size, user_pml4);
    user_task->mm = load_elf(elf, size, user_task, user_pml4);
    // Create space for a new stack
    uint64_t user_task_stack = (uint64_t) PHYS_TO_VIRT(kmalloc_pg());
    // Initialize the task
    create_new_elf_task(user_task, name, USER, NEUTRAL_PRIORITY, kernel_rflags,
                    user_pml4, user_task_stack, user_task->mm->start_code);
    // Add the task to the scheduler list
    insert_into_list(&tasks, user_task);
    // Print out contents of the task
    // dump_task(kernel_task);
    return user_task;

}

Task* create_user_task(const char *name, void(*code)()) {
    // Get the kernel page tables
    pml4_t *kernel_pml4 = (pml4_t *)get_cr3();
    // Copy the kernels page tables
    pml4_t *user_pml4 = copy_page_tables(kernel_pml4);
    // Get the kernel flags
    uint64_t kernel_rflags = get_rflags();
    // Allocate space for a new user task
    Task *user_task = (Task*) PHYS_TO_VIRT(kmalloc_pg());
    user_task->mm = NULL;
    // Create space for a new stack
    uint64_t user_task_stack = (uint64_t) PHYS_TO_VIRT(kmalloc_pg());
    // Initialize the task
    create_new_task(user_task, name, USER, NEUTRAL_PRIORITY, kernel_rflags,
                    user_pml4, user_task_stack, code);
    // Add the task to the scheduler list
    insert_into_list(&tasks, user_task);
    // Print out contents of the task
    // dump_task(kernel_task);
    return user_task;
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
    // dump_task(kernel_task);
    return kernel_task;
}


void setup_new_stack(Task *task) {
    uint64_t *stack = (uint64_t*)task->stack;
    // Set the segments for the correct ring
    if(task->type == KERNEL) {
        // printk("Kernel task\n");
        stack[511] = 0x10; // Set the SS
        stack[508] = 0x08; // Set the CS
    } else {
        // printk("User task\n");
        //stack[511] = 0x23; // Set the SS
        //stack[508] = 0x1b; // Set the CS
        stack[511] = 0x10; // Set the SS
        stack[508] = 0x08; // Set the CS
    }
    // Set the common stack values
    stack[510] = (uint64_t)&stack[511];  // set the top of the stack
    stack[509] = 0x200202;              // Set the flags
    stack[507] = task->registers.rip;   // The entry point
    // Set the stack pointer to the amount of items pushed
    task->registers.rsp = (uint64_t)&stack[507];
    // dump_task(task);

    // __asm__ __volatile__(
    //     /* Store the initial stack pointer */
    //     "movq %%rsp, %%rax;"
    //     "movq %1, %%rdi;"
    //     /* Set the stack pointer to the new tasks stack */
    //     "movq %2, %%rsp;"
    //     /* Push the instruction pointer onto the stack */
    //     "pushq %3;"
    //     /* Save the new value of the stack pointer */
    //     "movq %%rsp, 0x40(%%rdi);"
    //     /* Set the stack pointer back to what it was */
    //     "movq %%rax, %%rsp;"
    //     : "=r"(task)
    //     : "m"(task), "m"(task->registers.rsp), "m"(task->registers.rip)
    //     : "rax", "rdi"
    // );
}

Task* create_new_elf_task(Task* task, const char *name, task_type_t type,
                      priority_t priority, uint64_t flags, pml4_t *pml4,
                      uint64_t stack, uint64_t rip) {
    /* Set the basic task values */
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
    task->registers.rip = rip;
    task->registers.cr3 = (uint64_t)pml4;
    task->registers.rsp = stack + PAGE_SIZE - 8; // Start the stack pointer at the other side
    task->registers.rbp = task->registers.rsp;
    /* This task is the end of the list */
    task->next = NULL;
    task->prev = NULL;
    // Initialize the stack
    setup_new_stack(task);
    return task;
}


Task* create_new_task(Task* task, const char *name, task_type_t type,
                      priority_t priority, uint64_t flags, pml4_t *pml4,
                      uint64_t stack, void(*main)()) {
    /* Set the basic task values */
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
    setup_new_stack(task);
    return task;
}

int get_task_count(void) {   
    return task_count;
}

void preempt(bool discard) {
    Task *old_task = current_task;
    current_task = old_task->next;
    if(current_task == NULL) {
        // Set the current task back to the head of the list
        current_task = tasks;
    }
    if(discard) {
        // The old process no longer wants to run
        old_task->state = TERMINATED;
        task_count--;
    }
    // Attempt to switch tasks; Assembly magic voodo
    switch_tasks(old_task, current_task);
}

void set_task(Task *task) {
    current_task = task;
    current_task->state = RUNNING;
    /* Typicall this is used only one time for setting the first task */
    tss.rsp0 = task->registers.rip;
    /* Set the rest of the registers */
    __asm__ __volatile__(
        /* Save the argument in the register */
        "movq %0, %%rax;"
        /* Set cr3 */
        "movq 0x88(%0), %%rdi;"
        "movq %%rdi, %%cr3;"
        /* Set the rest of the registers */
        "movq 0x8(%%rax), %%rbx;"
        "movq 0x10(%%rax), %%rcx;"
        "movq 0x18(%%rax), %%rdx;"
        "movq 0x20(%%rax), %%rsi;"
        "movq 0x28(%%rax), %%rdi;"
        "movq 0x48(%%rax), %%r8;"
        "movq 0x50(%%rax), %%r9;"
        "movq 0x58(%%rax), %%r10;"
        "movq 0x60(%%rax), %%r11;"
        "movq 0x68(%%rax), %%r12;"
        "movq 0x70(%%rax), %%r13;"
        "movq 0x78(%%rax), %%r14;"
        "movq 0x80(%%rax), %%r15;"
        /* Set the base pointer */
        "movq 0x30(%%rax), %%rbp;"
        /* set the stack pointer */
        "movq 0x40(%%rax), %%rsp;"
        /* Set rax */
        "movq 0x0(%%rax), %%rax;"
        :
        : "r"(task)
        :
    );
}


void test(uint64_t value1, uint64_t value2) {
    printk("Value: %p %p\n", value1, value2);
    __asm__ __volatile__("cli; hlt;");
}

void switch_tasks(Task *old, Task *new) {
    // Make sure both are not null
    // and both are not the same (no need to swap if same)
    if(old != NULL && new != NULL && old != new) {
        if(old->state != TERMINATED) {
            old->state = READY;
            /* Save the current register state */
            __asm__ __volatile__(
                /* save rax so we can use it for scratch */
                "movq %%rax, 0x0(%0);"
                /* Save cr3 */
                "movq %%cr3, %%rax;"
                "movq %%rax, 0x88(%0);"
                /* Save flags */
                "pushfq;"
                "popq %%rax;"
                "movq %%rax, 0x90(%0);"
                /* Save the rest */
                "movq %%rbx, 0x8(%0);"
                "movq %%rcx, 0x10(%0);"
                "movq %%rdx, 0x18(%0);"
                "movq %%rsi, 0x20(%0);"
                "movq %%rdi, 0x28(%0);"
                "movq %%r8, 0x48(%0);"
                "movq %%r9, 0x50(%0);"
                "movq %%r10, 0x58(%0);"
                "movq %%r11, 0x60(%0);"
                "movq %%r12, 0x68(%0);"
                "movq %%r13, 0x70(%0);"
                "movq %%r14, 0x78(%0);"
                "movq %%r15, 0x80(%0);"
                /* save rbp */
                "movq %%rbp, 0x30(%0);"
                /* save rsp */
                "movq %%rsp, 0x40(%0);"
                : 
                : "r"(old)
                :
            );
        } else {
            // If the old task has terminated, clean it up
            if(remove_task_by_pid(&tasks, old->pid) == NULL) {
                panic("Tried to free a NULL task.\n");
                __asm__ __volatile__("cli; hlt;");
            }
            printk("Removed: %s\n", old->name);
            // Mark pid as free
            free_pid(old->pid);
            // Free the stack
            kfree_pg((void*)(old->stack));
            // Free the task struct
            kfree_pg((void*)old);
        }
        // Now set the new task to run
        current_task = new;
        __asm__ __volatile__(
            /* Save the argument in the register */
            "movq %0, %%rax;"
            /* Set cr3 */
            "movq 0x88(%0), %%rdi;"
            "movq %%rdi, %%cr3;"
            /* Set the rest of the registers */
            "movq 0x8(%%rax), %%rbx;"
            "movq 0x10(%%rax), %%rcx;"
            "movq 0x18(%%rax), %%rdx;"
            "movq 0x20(%%rax), %%rsi;"
            "movq 0x28(%%rax), %%rdi;"
            "movq 0x48(%%rax), %%r8;"
            "movq 0x50(%%rax), %%r9;"
            "movq 0x58(%%rax), %%r10;"
            "movq 0x60(%%rax), %%r11;"
            "movq 0x68(%%rax), %%r12;"
            "movq 0x70(%%rax), %%r13;"
            "movq 0x78(%%rax), %%r14;"
            "movq 0x80(%%rax), %%r15;"
            /* Set the base pointer */
            "movq 0x30(%%rax), %%rbp;"
            /* set the stack pointer */
            "movq 0x40(%%rax), %%rsp;"
            /* Set rax */
            "movq 0x0(%%rax), %%rax;"
            :
            : "r"(current_task)
            : "memory"
        );

        if(current_task->state == NEW) {
            printk("Task Name: %s\n", current_task->name);
            // __asm__ __volatile__("retq;");
            __asm__ __volatile__("iretq;");
        } else {
            current_task->state = RUNNING;
            // Check to see if the task being scheduled is user or kernel
            if(current_task->type == USER) {
                // Need to set the tss rsp0 value
//                tss.rsp0 = (uint64_t)&(((uint64_t*)current_task->stack)[511]);
//                SWITCH_TO_RING3();
            }
        }
    }
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
    if(success) task_count++;
    return success;
}

Task *get_task_by_pid(Task **list, pid_t pid) {
    Task *task = NULL;
    if(list != NULL && *list != NULL && pid >= 0) {
        Task *ctask = *list;
        while(ctask != NULL) {
            if(ctask->pid == pid) {
                task = ctask;
                break;
            } else {
                ctask = ctask->next;
            }
        }
    }
    return task;
}

Task *remove_task_by_pid(Task **list, pid_t pid) {
    Task *task = NULL;
    if(list != NULL && *list != NULL && pid >= 0) {
        Task *ctask = *list;
        while(ctask != NULL) {
            if(ctask->pid == pid) {
                // found it!
                task = ctask;
                if(task->prev == NULL) {
                    // HEAD of the list
                    *list = task->next;
                } else {
                    // Some other part of the list
                    task->prev->next = task->next;
                    if(task->next != NULL) {
                        task->next->prev = task->prev;
                    }
                }
                break;
            } else {
                ctask = ctask->next;
            }
        }
    }
    return task;
}

Task* get_current_task(void) {
    return current_task;
}
