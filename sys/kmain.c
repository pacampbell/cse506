#define __KERNEL__
#include <sbunix/kmain.h>

void idle(void) {
    // TODO: fix this
    while(1) {
        __asm__ __volatile__("hlt;");
        preempt(false);
    } 
    preempt(true);
}

uint64_t *kstack;
uint64_t *kstack_top;

/**
 * Similar to initd, this is process 0. This starts up all kernel level 
 * services and threads. 
 */
void kmain(void) {
    /* Create a stack for handling system calls */
    kstack = (uint64_t*)PHYS_TO_VIRT(kmalloc_pg());
    kstack_top = &kstack[511];
    // printk("kstack: %p kstack_top: %p\n", kstack, kstack_top);
    /* do some basic setup */
    init_services();
    /* start the shell */
    start_shell();
    /* Everything is started now spin */
    while(1) {
        // printk("\n ==== kmain ====\n");
        /* Done doing our work, now just wait */
        preempt(false);
    }
}

void init_services(void) {
    /* WARNING */
    /* If you change the order you should update constants in <sys/task.h> */
    create_kernel_task("idle", idle);           /* Should be pid 1 */
    /* END WARNING */
}

void start_shell(void) {
    /* Do some testing for now */
    // exec_tarfs_elf("bin/ps");
    exec_tarfs_elf("bin/hello");
    exec_tarfs_elf("bin/ps");
    //exec_tarfs_elf("bin/ps");
    // exec_tarfs_elf("bin/mike");
    // exec_tarfs_elf("bin/sh");
    // exec_tarfs_elf("bin/ps");
}
