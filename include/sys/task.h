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
        #include <sys/mm/vma.h>
        #include <sys/gdt.h> 
        #include <sys/fs/file.h>
        /* Process/Task constants and typedefs */
        #define PID_MAP_LENGTH 65535
        #define MAX_PRIORITY 0xFFFFFFFF
        #define NEUTRAL_PRIORITY 0x7FFFFFFF
        #define MIN_PRIORITY 0x00000000
        /* Known ktask ids */
        #define KMAIN_PID 0
        #define IDLE_PID 1
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
                     rflags,
                     cs, ss;
        };
        typedef struct Registers Registers;

        struct args_struct {
            uint64_t argc;
            uint64_t argv;
            uint64_t envp;
        };

        struct Task {
            Registers registers;                /* Struct containing all the IA-64 registers */
            uint64_t *kstack;                   /* Address of the kernel stack */
            uint64_t *ustack;                   /* Address of the user stack */
            task_state_t state;                 /* State of the process */
            pid_t pid;                          /* Unique identifier of the process */
            const char *name;                   /* Convenience name; Optional */
            priority_t priority;                /* priority value from [0, 2^64 - 1] */
            task_type_t type;                   /* Type of task kernel or user */
            ssize_t sleep;                      /* -1 if task is not sleeping */
            bool is_yield;                      /* Flag determining if this process is currently in a yielding state */
            bool in_use;                        /* Flag determing if the struct is in use or not */
            struct Task *parent;                /* Pointer to the parent of this task */
            struct Task *children;              /* Head pointer to list of children */
            struct Task *next;                  /* Next task in the list */
            struct Task *prev;                  /* Previous Task in the list */
            struct mm_struct *mm, *active_mm;   /* The mm_struct of this task */
            struct file *files[MAX_FD];
            struct args_struct args;
            char cwd[MAX_PATH];
        };
        typedef struct Task Task;

        /* privilege macros */
        #define SWITCH_TO_RING3() do {  \
            __asm__ __volatile__(       \
                "movq %%rsp, %%rax;"    \
                "pushq $0x23;"          \
                "pushq %%rax;"          \
                "pushq $0x200;"         \
                "pushq $0x1b;"          \
                "pushq $1f;"            \
                "iretq;"                \
                "1:;"                   \
                :                       \
                :                       \
                :                       \
            );                          \
        } while(0)

        /* Kernel threads tasks */

        /**
         * Generic function which creates a specified task.
         */
        Task* create_new_task(Task* task, const char *name, task_type_t type, priority_t priority, uint64_t flags, pml4_t *pml4, void(*main)());
        // Task* create_new_elf_task(Task* task, const char *name, task_type_t type, priority_t priority, uint64_t flags, pml4_t *pml4, uint64_t stack, uint64_t rip); 

        /**
         * Create a kernel task.
         * @param name Short name identifying the name of the task.
         * @param code Function pointer which should start at the code to be executed.
         */
        Task* create_kernel_task(const char *name, void(*code)());

        /**
         * Create a user task.
         * @param name Short name identifying the name of the task.
         * @param code Function pointer which should start at the code to be executed.
         */
        Task* create_user_elf_task(const char *name, char* elf, uint64_t size);

        /**
         * Searchs the current task list for any unused task structs.
         * if none exist then create a new one otherwise we return the address
         * of an existing unused task struct.
         */
        Task* create_task_struct(void);

        /**
         * Search the task list for a free task if it exists.
         * @return Returns NULL if there are no free tasks otherwise 
         * it returns the address of a free task.
         */
        Task* get_free_task_struct(void);

        /**
         * Pushes values into the stack for the newly created task.
         * @param task Task stack to initialize.
         */
        void setup_new_stack(Task *task);

        /**
         * get the number of running tasks in the list
         */
        int get_task_count(void);

        /**
         * Dumps the contents of a task.
         */
        void dump_task(Task *task);
        /**
         * Sets the current task to be ran.
         */
        void set_task(Task *task);

        void run_task(Task *task);
        // Switch tasks
        void switch_tasks(Task *old, Task *new, bool use_global);
        // preempt
        void preempt(bool discard, bool use_global);
        // Methods for getting tasks
        bool insert_into_list(Task *task);
        Task *get_task_by_pid(pid_t pid);
        Task *remove_task_by_pid(pid_t pid);
        Task *get_current_task(void);
        Task *clone_task(Task *src, uint64_t global_sp, uint64_t global_rip);
        Task *get_task_list(void);
        Task* create_user_elf_args_task(const char *name, char* elf, uint64_t size, int argc, char *argv[], char *envp[]);

        /**
         * Get the next valid task that is in use in the task list.
         * If no valid tasks exist return NULL.
         */
        Task *get_next_task(void);
        /* TODO: Future work - Forking functions should go in here too? */
    #endif
#endif
