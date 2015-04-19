#define __KERNEL__
#include <sbunix/kmain.h>
#include <sys/sbunix.h>
#include <sys/task.h>

void kmain(void) {
    printk("In kmain!\n");
    /* Now that we are in a somewhat stable state initialize tasks for kernel */
    initialize_tasking();
}
