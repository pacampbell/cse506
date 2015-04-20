#define __KERNEL__
#include <sbunix/kmain.h>
#include <sys/sbunix.h>
#include <sys/task.h>

void awesomefunc() {
    printk("Scheduled task!!! woot woot");
}

void kmain(void) {
    printk("In kmain!\n");
    void initialize_scheudler();
    /* Now that we are in a somewhat stable state initialize tasks for kernel */
    initialize_task("awesomefunc", 0xffffffffffffffff, awesomefunc);
    /* Now schedule the task! */
    // preempt();
    /* Hopefully we get here eventually! */
    printk("Back in the kernel!\n");
}
