#define __KERNEL__
#include <sbunix/kmain.h>
#include <sys/sbunix.h>
#include <sys/task.h>

void awesomefunc(void) {
    printk("Scheduled task!!! woot woot\n");
    preempt();
}

void idle(void) {
    while(1) {
        printk("idle loop\n");
        preempt();
    } 
}

void kmain(void) {
    printk("In kmain!\n");
    // Task *idle_task = 
    create_kernel_task("idle", idle);
    // set_task(idle_task);
    // dump_task(idle_task);
    // create_kernel_task("awesomefunc", awesomefunc);
    /* Now schedule the task! */
    printk("Before kmain preempt!\n");
    preempt();
    /* Hopefully we get here eventually! */
    printk("Back in the kernel!\n");
    // preempt();
    while(1);
}
