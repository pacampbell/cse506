#define __KERNEL__
#include <sys/task.h>
#include <sys/elf.h>
#include <sys/screen.h>
#include <sbunix/debug.h>

static Task *tasks;
static Task *current_task = NULL;
static Task *prev_task = NULL;
static uint64_t task_count = 0;

/* Used for assigning a pid to a task */
static uint64_t pid_map[PID_MAP_LENGTH];

uint64_t amazing_bug_fixer = 0;

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

void dump_mm(struct mm_struct *mm) {
    if (mm != NULL) {
        printk(" me: %p\n", mm);
        printk(" mm->start_stack: %p\n", mm->start_stack);
        printk(" mm->mmap_base  : %p\n", mm->mmap_base);
        printk(" mm->brk        : %p\n", mm->brk);
        printk(" mm->start_brk  : %p\n", mm->start_brk);
        printk(" mm->end_data   : %p\n", mm->end_data);
        printk(" mm->start_data : %p\n", mm->start_data);
        printk(" mm->end_code   : %p\n", mm->end_code);
        printk(" mm->start_code : %p\n", mm->start_code);

    } else {
        printk("MM WAS NULL\n");
    }
}

void dump_vma(struct vm_area_struct *vma) {
    if (vma != NULL) {
        printk("==============VMA=================\n");
        printk("vma->prev     : %p\n", vma->prev);
        printk("vma->vm_start : %p\n", vma->vm_start);
        printk("vma->vm_prot  : %p\n", vma->vm_prot);
        printk("vma->vm_mm    : %p\n", vma->vm_mm);
        printk("vma->vm_end   : %p\n", vma->vm_end);
        printk("vma->next     : %p\n", vma->next);
        printk("me            : %p\n", vma);

        dump_vma(vma->next);

    }
}

/**
 * Prints out the values of a task
 * @param task Task to display values of.
 */
void dump_task(Task *task) {
    #ifdef DEBUG
//#if 1
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
            printk("kstack: %p ustack: %p\n", task->kstack, task->ustack);
            printk("kstack-lim: %p ustack-lim: %p\n", &task->kstack[511], &task->ustack);
            printk("previous: %p next: %p\n", task->prev, task->next);
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

typedef void (*code)();
      
Task* create_user_elf_task(const char *name, char* elf, uint64_t size) {
    // Get the kernel page tables
    pml4_t *kernel_pml4 = (pml4_t *)get_cr3();
    // Copy the kernels page tables
    pml4_t *user_pml4 = copy_page_tables(kernel_pml4);
    // Allocate space for a new user task
    Task *user_task = create_task_struct(&tasks);
    //user_task->mm = load_elf(elf, size, user_pml4);
    user_task->mm = load_elf(elf, size, user_task, user_pml4);
    // Initialize the task
    create_new_task(user_task, name, USER, NEUTRAL_PRIORITY, 0, user_pml4, 
                   (code)user_task->mm->start_code);
    // Add the task to the scheduler list
    insert_into_list(user_task);
    return user_task;

}

Task* create_kernel_task(const char *name, void(*code)()) {
    // Get the kernel page tables
    pml4_t *kernel_pml4 = (pml4_t *)get_cr3();
    // Get the kernel flags
    uint64_t kernel_rflags = get_rflags();
    // Allocate space for a new kernel task
    Task *kernel_task = create_task_struct(&tasks);
    // Kernel tasks have no mm so just make it null
    kernel_task->mm = NULL;
    // Initialize the task
    create_new_task(kernel_task, name, KERNEL, MAX_PRIORITY, kernel_rflags,
                    kernel_pml4, code);
    // Add the task to the scheduler list
    insert_into_list(kernel_task);
    // Print out contents of the task
    // dump_task(kernel_task);
    return kernel_task;
}

Task* create_task_struct(Task **list) {
    Task *task = get_free_task_struct(list);
    if(task == NULL) {
        // There was no more free tasks so create a new one
        task = (Task*) PHYS_TO_VIRT(kmalloc_pg());
        memset(task, 0, sizeof(Task));
    } else {
        // Remove this task from the insert_into_list
        free_file_list(task->files, MAX_FD);
        if(remove_task_by_pid(list, task->pid) == NULL) {
            panic("Tried to free a NULL task.\n");
            __asm__ __volatile__("cli; hlt;");
        }
    }
    return task;
}

Task *get_next_task(void) {
    Task *ctask = current_task->next;
    if(ctask == NULL) {
        // Set back to the head of the list
        ctask = tasks;
    }
    // Now find the next in-use task struct
    while(ctask != NULL && !ctask->in_use) {
        ctask = ctask->next;
    }
    return ctask;
}

void setup_new_stack(Task *task) {
    // uint64_t *stack = task->type == USER ? task->ustack : task->kstack;
    // Set the segments for the correct ring
    if(task->type == KERNEL) {
        // printk("Kernel task\n");
        task->kstack[511] = 0x10; // Set the SS
        task->kstack[508] = 0x08; // Set the CS
    } else {
        // printk("User task\n");
        task->kstack[511] = 0x23; // Set the SS
        task->kstack[508] = 0x2b; // Set the CS
    }
    // set the top of the user/kernel stack
    task->kstack[510] = task->registers.rsp;  // (uint64_t)&(stack[511]);
    task->kstack[509] = 0x202;                // Set the flags
    task->kstack[507] = task->registers.rip;  // The entry point (on a forked task this might not be the start)
    // Set the stack pointer to the amount of items pushed
    task->registers.rsp = (uint64_t)&(task->kstack[507]);
}

Task* create_new_task(Task* task, const char *name, task_type_t type,
                      priority_t priority, uint64_t flags, pml4_t *pml4,
                      void(*main)()) {
    /* Set the basic task values */
    task->name = name;
    task->priority = priority;
    task->state = NEW;
    task->type = type;
    task->pid = allocate_pid();
    task->in_use = true;
    /* Set the address of the stack */
    task->kstack = (uint64_t*) PHYS_TO_VIRT(kmalloc_pg());
    if(type == USER) {
        // panic("Allocate user stack\n");
        //task->ustack = (uint64_t*)kmalloc_vma(pml4, VIRTUAL_OFFSET, PAGE_SIZE, USER_SETTINGS);
        uint64_t new_stack = task->mm->brk + (50 * PAGE_SIZE);
        new_stack &= PG_ALIGN;

        // pdpt_t *pml4e = get_pml4e(pml4, new_stack); 
        // if(pml4e == NULL) {
        //     printk("pml4e is null (good)\n");
        // } else {
        //     printk("pml4e is not null: Mistake?\n");
        // }
        uint64_t page = get_pte(pml4, new_stack);
        if(page != 0x0) {
            printk("page already exists @ %p. Mistake? page: %p\n", new_stack, page);
        } else {
            printk("A page does not exist @ %p\n", new_stack);
        }
        if(kmalloc_vma(pml4, new_stack, 1, USER_SETTINGS) == NULL) {
            panic("KMALLOC VMA FAILED\n");
        }
        task->ustack = (uint64_t*)(new_stack);

        page = get_pte(pml4, new_stack);
        if(page != 0x0) {
            printk("page exists @ %p. page: %p\n", new_stack, page);
        } else {
            printk("A page does not exist @ %p Mistake?\n", new_stack);
            pt_t *pt = get_pde(pml4, new_stack);
            if(pt == NULL) {
                printk("The page table does not exist for %p\n", new_stack);
                pd_t *pd = get_pdpte(pml4, new_stack);
                if(pd == NULL) {
                    printk("Page directory does not exist for %p\n", new_stack);
                    pdpt_t *pdpt = get_pml4e(pml4, new_stack);
                    if(pdpt == NULL) {
                        printk("Page directory pointer table does not exist for %p\n", new_stack);
                    } else {
                        printk("Page directory pointer table exists! %p\n", pdpt);
                    }
                } else {
                    printk("Page directory exists! %p\n", pd);
                }
            } else {
                printk("Page table exists! %p\n", pt);
            }
        }

        // halt();
    } else {
        task->ustack = 0;
    }
    /* Zero out the general purpose registers */
    task->registers.rax = 0;
    task->registers.rbx = 0;
    task->registers.rcx = 0;
    task->registers.rdx = 0;
    task->registers.rsi = 0;
    task->registers.rdi = 0;
    task->registers.r8  = 0;
    task->registers.r9  = 0;
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
    task->registers.rsp = type == KERNEL ? (uint64_t)task->kstack + PAGE_SIZE - 8 : (uint64_t)task->ustack + PAGE_SIZE - 8; // Start the stack pointer at the other side
    task->registers.rbp = task->registers.rsp;
    // printk("stack base: %p\n", task->registers.rbp);

    if(task->mm != NULL) {
        task->mm->start_stack = task->registers.rsp;
    }

    /* This task is the end of the list */
    task->next = NULL;
    task->prev = NULL;
    /* Set parent and children to nothing */
    task->parent = NULL;
    task->children = NULL;
    // Initialize the stack
    setup_new_stack(task);
    return task;
}

extern pml4_t *g_kernel_pgtable;
Task *clone_task(Task *src, uint64_t global_sp, uint64_t global_rip) {
    Task *new_task = NULL;
    if(src != NULL) {
        // Get a page for a new task struct
        new_task = create_task_struct(&tasks);
        // zero out the struct
        memset(new_task, 0, sizeof(Task));
        // Copy the struct (no deep copies)
        memcpy(new_task, src, sizeof(Task));
        // Copy the page tables of the source process
        // pml4_t *current_pml4 = get_cr3();
        // set_cr3(g_kernel_pgtable);
        printk("Start of user code: %p\n", src->mm->start_code);
        pml4_t *cloned_pml4 = copy_page_tables((pml4_t*)(src->registers.cr3));
        // Set copied cr3
        new_task->registers.cr3 = (uint64_t)cloned_pml4;
        // Assign a new pid
        new_task->pid = allocate_pid();
        // Assign the parent task
        new_task->parent = src;
        // Make sure this task is still valid
        new_task->in_use = true;
        // Assign the child task return value to zero
        new_task->registers.rax = 0;
        // Expliciting say this program is ready
        new_task->state = READY;
        // Set some pointers to be NULL
        new_task->next = NULL;
        new_task->prev = NULL;
        new_task->children = NULL;
        // Create new kstack and ustack
        new_task->kstack = (uint64_t*) PHYS_TO_VIRT(kmalloc_pg());
        memset(new_task->kstack, 0, PAGE_SIZE);        
        // Create new user stack
        if(new_task->type == USER) {
            new_task->ustack = (uint64_t*)kmalloc_vma((pml4_t*)(new_task->registers.cr3), 
                                                      new_task->mm->start_stack, PAGE_SIZE, USER_SETTINGS);
            if(new_task->ustack == NULL) {
                panic("KMALLOC VMA FAILED\n");
            }
            // copy the parents user stack
            memcpy(new_task->ustack, src->ustack, PAGE_SIZE);
        }
        // Setup the stack again
        setup_new_stack(new_task);
        // set_cr3(current_pml4);
    } else {
        panic("UNABLE TO CLONE NULL TASK");
        halt();
    }
    return new_task;
}

int get_task_count(void) {   
    return task_count;
}

void preempt(bool discard) {
    Task *old_task = current_task;
    current_task = get_next_task();

    if(current_task == NULL) {
        panic("NEXT TASK RETURNED NULL. This should NEVER HAPPEN\n");
        halt();
    }

    // printk("\n==== NEXT - %s ====\n", current_task->name);

    if(discard) {
        // The old process no longer wants to run
        old_task->state = TERMINATED;
        task_count--;
        // printk("Removing: pid: %d name: %s\n", old_task->pid, old_task->name);
    }
    // Attempt to switch tasks; Assembly magic voodo
    switch_tasks(old_task, current_task);
}

void set_task(Task *task) {
    // printk("==== ENTRY POINT - %s ====\n", task->name);
    current_task = task;
    current_task->state = RUNNING;
    /* Typically this is used only one time for setting the first kernel task */
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

void switch_tasks(Task *old, Task *new) {
    //if(amazing_bug_fixer++ == 1) halt ();
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
            // Mark pid as free
            free_pid(old->pid);
            // Mark the task struct as not in use
            old->in_use = false;
            // printk("old has been released. %p\n", old);
        }
        // Save the previosu task so we can check a few fields
        prev_task = current_task;
        // Now set the new task to run
        current_task = new;
        // Now swap to new task
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
            // printk("prev name: %s\n", prev_task->name);
            // printk("this name: %s\n", current_task->name);
            // Set the current task to a running state
            current_task->state = RUNNING;
            static bool first = true; 
            if((current_task->type == USER && first) || (current_task->type == USER && prev_task->type == KERNEL)) {
                first = false;
                tss.rsp0 = (uint64_t)&((current_task->kstack)[511]);
                current_task->state = RUNNING;
               
                __asm__ __volatile__(
                    "movq $0x23, %%rax;"
                    "movq %%rax, %%ds;"
                    "movq %%rax, %%es;"
                    "movq %%rax, %%fs;"
                    "movq %%rax, %%gs;"
                    
                    "movq $0x33, %%rax;" // Since we added 64-bit gdt entry, tss moved by 1 spot 
                    "ltr %%ax;"
                    // "pop %%rax;"
                    "iretq;"
                    :
                    :
                    :
                );
            } else {
                // dump_task(current_task);
                // dump_mm(current_task->mm);
                // printk("name: %s\n", current_task->name);
                // Just iretq
                __asm__ __volatile__("iretq;");
            }
        } else {
            current_task->state = RUNNING;
            // Check to see if the task being scheduled is user or kernel
            // if(current_task->type == USER && prev_task->type == KERNEL) {
            //     printk("Switching rings on user task\n");
            //     halt();
            //     __asm__ __volatile__("iretq;");
            // }
            if(current_task->type == USER) {
                // extern uint64_t global_sp;
                // extern uint64_t global_rip;
                printk("Continuing a started task: %s %d %d\n", current_task->name, current_task->pid, current_task->parent->pid);
                BOCHS_MAGIC();
                __asm__ __volatile__(
                    // "pushq 0x23;"
                    // "pushq %0;"
                    // "pushq 0x202;"
                    // "pushq 0x2b;"
                    // "pushq %1;"
                    "add $0x8, %%rsp;"
                    "iretq;"
                    :
                    : // "r"(global_sp), "r"(global_rip)
                    :
                );
            }
        }
    }
}

bool insert_into_list(Task *task) {
    bool success = false;
    if(task != NULL) {
        if(tasks == NULL) {
            // First task to be added to the list
            tasks = task;
            task->prev = NULL;
            task->next = NULL;
        } else {
            // Just set the old head to the next field of the new list
            task->next = tasks;
            task->prev = NULL;      // This new task is now the head so its previous is NULL.
            tasks->prev = task;     // Set the old heads previous to the new head
            // Set the head of the list to the new task
            tasks = task;
            // Mark the operation as a success
        }
        success = true;
    } else {
        panic("FAILED TO INSERT TASK INTO LIST\n");
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

Task* get_free_task_struct(Task **list) {
    Task *free_task = NULL;
    if(list != NULL && *list != NULL) {
        Task *ctask = *list;
        while(ctask != NULL) {
            if(!ctask->in_use) {
                free_task = ctask;
                // zero out the free task
                memset(free_task, 0, sizeof(Task));
                break;
            } else {
                ctask = ctask->next;
            }
        }
    }
    return free_task;
}

Task* get_current_task(void) {
    return current_task;
}

Task *get_task_list(void) {
    return tasks;
}
