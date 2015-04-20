#ifndef __KERNEL__
#error "Trying to include kernel code in non-kernel code."
#else
    #ifndef _TASK_H
    #define _TASK_H
        /* FIXME: This file only defines kernel multitasking as of now */
        /* FIXME: Userspace multitasking still needs to be implemented */
        #include <sys/defs.h>
        #include <sbunix/string.h>
        #include <sbunix/pgtable.h>
        /* Process/Task constants and typedefs */
        #define STACK_SIZE 2048
        #define PID_MAP_LENGTH 65535
        #define MAX_PRIORITY 0xFFFFFFFF
        #define MIN_PRIORITY 0x00000000
        /* Enum for keeping state of the process */
        typedef enum {
            NEW,         /* The process is in the stage of being created */
            READY,       /* The process has all the resources available that it needs to run, but the CPU is not currently working on this process's instructions */
            RUNNING,     /* The CPU is working on this process's instructions */
            WAITING,     /* The process cannot run at the moment, because it is waiting for something */
            TERMINATED   /* The process has completed */
        } task_state_t;
        /* Typedef for the processes unique id */
        typedef ssize_t pid_t;
        /* Typedef for the process priority */
        typedef uint64_t priority_t;
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
            Registers registers;                /* Struct containing all the IA-64 registers */
            unsigned char  stack[STACK_SIZE];   /* Array containing the statck of the process */
            task_state_t state;                 /* State of the process */
            pid_t pid;                          /* Unique identifier of the process */
            const char *name;                   /* Convenience name; Optional */
            priority_t priority;                /* priority value from [0, 2^64 - 1]*/
            struct Task *next;                  /* Next task in the list */
            struct Task *prev;                  /* Previous Task in the list */
        };
        typedef struct Task Task;

        /* Define functions for creating and managing kernel level tasks */
        void initialize_scheudler(void(*main)()); // Sets the kernel as the first task in the scheduler
        // initialize task
        void initialize_task(const char *name, priority_t priority, void(*main)());
        // Create a task
        void create_new_task(Task* task, void(*main)(), const char *name, priority_t priority, uint64_t flags, uint64_t *pml4);
        // Switch tasks
        void switch_tasks(Task *old, Task *new);
        // preempt
        void preempt(void);
        // Methods for getting tasks
        bool insert_into_list(Task **list, Task *task);
        Task *get_task_by_pid(pid_t pid);
        Task *remove_task_by_pid(pid_t pid);
        /* TODO: Future work - Forking functions should go in here too? */
    #endif
#endif
