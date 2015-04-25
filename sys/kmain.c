#define __KERNEL__
#include <sbunix/kmain.h>
#include <sys/sbunix.h>
#include <sys/task.h>
#include <sys/tarfs.h>

void awesomefunc(void) {
    printk("I'm awesome!!! woot woot\n");
    preempt(true);
}

void idle(void) {
    while(1) {
        printk("idle loop\n");
        preempt(false);
    } 
}

void kmain(void) {
    traverse_tars();
    while(1);
    // create_kernel_task("awesomefunc", awesomefunc);
    // create_kernel_task("idle", idle);
    /* Let something else run! */
    // preempt(false);
    /* We are back... now what? */
    // while(1) {
        /* Hopefully we get here eventually! */
        // printk("In kmain!\n");
        // preempt(false);
    //}
}
