#define __KERNEL__
#include <sys/task.h>

extern void printk();

// static Task main_kernel_task;
// static Task *tasks;
static Task *current_task;

/* Used for assigning a pid to a task */
static uint64_t pid_map[PID_MAP_LENGTH];

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

static void free_pid(pid_t pid) {
    // Make sure we didn't get a bad pid
    if(pid >= 0){
        // Convert the pid into a position in the pid map
        size_t array_index = pid / 64;
        size_t bit_mask = ~(1 << (pid % 64));
        pid_map[array_index] &= bit_mask;
    }
}

void initialize_tasking(void) {

}

void create_new_task(Task* task, void(*main)(), uint64_t flags, uint64_t *pml4) {
    task->pid = allocate_pid(); // FIXME: Generate a pid
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
    task->registers.rip = (uint64_t)0; // FIXME: Need to set this to the stack address
    /* This task is the end of the list */
    task->next = NULL;
}

void preempt(void) {
    // Task *task = current_task;
    current_task = current_task->next;
    // switch_tasks(&task->registers, &current_task->registers);
}

void switch_tasks(Task *old, Task *new) {

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
