#define __KERNEL__
#include <sbunix/kmain.h>
#include <sys/sbunix.h>
#include <sys/task.h>

void awesomefunc() {
    printk("Scheduled task!!! woot woot\n");
    preempt();
}

void idle() {
    while(1) {
        printk("idle loop\n");
        preempt();
    } 
}

void kmain(void) {
    printk("In kmain!\n");
    create_kernel_task("idle", idle);
    create_kernel_task("awesomefunc", awesomefunc);
    /* Now schedule the task! */
    preempt();
    /* Hopefully we get here eventually! */
    printk("Back in the kernel!\n");
    while(1);
}
