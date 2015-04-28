#define __KERNEL__
#include <sbunix/kmain.h>

void awesomefunc(void) {
    printk("I'm awesome!!! woot woot\n");
    preempt(true);
}

void idle(void) {
    while(1) {
        // printk("idle loop\n");
        preempt(false);
    } 
}

/**
 * Similar to initd, this is process 0. This starts up all kernel level 
 * services and threads. 
 */
void kmain(void) {
    /* do some basic setup */
    init_services();
    /* start the shell */
    start_shell();
    /* Everything is started now spin */
    while(1) {
        /* Done doing our work, now just wait */
        preempt(false);
    }
}

void init_services(void) {
    // create_kernel_task("awesomefunc", awesomefunc);
    create_kernel_task("idle", idle); /* Create the kernel idle loop */
}

void start_shell(void) {
    /* Do some testing for now */
    tarfs_entry e;
    if(traverse_tars("bin/hello", &e) != NULL) {
        printk("name: %s\nLocation: %p\nSize: %d\n", e.path, e.data_base, e.size);
        /* Try to load file as elf */
        load_elf(e.data_base, e.size);
    } else {
        printk("Unable to find: %s in tarfs\n", "bin/hello");
    }
}
