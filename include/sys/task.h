#ifndef __KERNEL__
#error "Trying to include kernel code in non-kernel code."
#else
    #ifndef _TASK_H
    #define _TASK_H
        /* FIXME: This file only defines kernel multitasking as of now */
        /* FIXME: Userspace multitasking still needs to be implemented */
        #include <sys/defs.h>
        #include <sbunix/string.h>
        #include <sys/pgtable.h>
        /* Process/Task constants and typedefs */
        #define PID_MAP_LENGTH 65535
        #define MAX_PRIORITY 0xFFFFFFFF
        #define NEUTRAL_PRIORITY 0x7FFFFFFF
        #define MIN_PRIORITY 0x00000000
        /* Enum for keeping state of the process */
        typedef enum {
            NEW,         /* The process is in the stage of being created */
            READY,       /* The process has all the resources available that it needs to run, but the CPU is not currently working on this process's instructions */
            RUNNING,     /* The CPU is working on this process's instructions */
            WAITING,     /* The process cannot run at the moment, because it is waiting for something */
            TERMINATED   /* The process has completed */
        } task_state_t;
        /* Enum for tracking the type of process */
        typedef enum {
            KERNEL,
            USER
        } task_type_t;
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
            uint64_t stack;                     /* Address of your stack */
            task_state_t state;                 /* State of the process */
            pid_t pid;                          /* Unique identifier of the process */
            const char *name;                   /* Convenience name; Optional */
            priority_t priority;                /* priority value from [0, 2^64 - 1]*/
            task_type_t type;                   /* Type of task kernel or user */
            struct Task *next;                  /* Next task in the list */
            struct Task *prev;                  /* Previous Task in the list */
        };
        typedef struct Task Task;

        /* Kernel threads tasks */

        /**
         * Generic function which creates a specified task.
         */
        Task* create_new_task(Task* task, const char *name, task_type_t type, priority_t priority, uint64_t flags, pml4_t *pml4, uint64_t stack, void(*main)());

        /**
         * Create a kernel task.
         * @param name Short name identifying the name of the task.
         * @param code Function pointer which should start at the code to be executed.
         */
        Task* create_kernel_task(const char *name, void(*code)());

        /**
         * Pushes values into the stack for the newly created task.
         * @param task Task stack to initialize.
         */
        void setup_new_stack(Task *task);

        /**
         * Same as setup_new_stack except it doesn't push the 
         * rip value onto the stack.
         */
        void prepare_stack(Task *task);

        /**
         * Dumps the contents of a task.
         */
        void dump_task(Task *task);
        /**
         * Sets the current task to be ran.
         */
        void set_task(Task *task);
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
