#ifndef __KERNEL__
#error "Trying to include kernel code in non-kernel code."
#else
    #ifndef _TASK_H
    #define _TASK_H
        /* FIXME: This file only defines kernel multitasking as of now */
        /* FIXME: Userspace multitasking still needs to be implemented */
        #include <sys/defs.h>
        /* Define types for keeping track of a task */
        struct Registers {
            uint64_t rax, rbx, rcx, rdx,
                     rsi, rdi,
                     rbp, rip, rsp,
                     r8, r9, r10, r11, r12, r13, r14, r15,
                     cr3,
                     rflags;
        };
        typedef struct Registers Registers;

        struct Task {
            Registers registers;
            struct Task *next;
        };
        typedef struct Task Task;

        /* Define functions for creating and managing kernel level tasks */
        // initialize a task
        void initialize_tasking(void);
        // Create a task
        void create_new_task(Task* task, void(*main)(), uint64_t flags, uint64_t *pml4);
        // Switch tasks
        void switch_tasks(Registers *old, Registers *new);
        // preempt
        void preempt(void);

        /* TODO: Future work - Forking functions should go in here too? */
    #endif
#endif
