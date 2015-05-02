#define __KERNEL__
#include <sbunix/kmain.h>

void awesomefunc(void) {
    int a = 1000;
    while(a--) {
        // printk("I'm awesome!!! woot woot %d\n", a);
        preempt(false);
    }
    preempt(true);
}

void idle(void) {
    // TODO: fix this
    while(1) {
        __asm__ __volatile__("hlt;");
        if(get_task_count() > 3) {
            //break;
        } 
        preempt(false);
    } 
    preempt(true);
}

/**
 * Similar to initd, this is process 0. This starts up all kernel level 
 * services and threads. 
 */
void kmain(void) {
    /* do some basic setup */
    init_services();
    /* start the shell */
    // start_shell();
    /*
    char *str = "Test?";
    __asm__ __volatile__(
        "movq $1, %%rax;"
        "movq $1, %%rdi;"
        "movq %0, %%rsi;"
        "movq $5, %%rdx;"
        "syscall;" 
        : 
        : "r"(str)
        : "rax"
    );
    */
    /* Everything is started now spin */
    while(1) {
        //TODO: fix this
        if(false && get_task_count() <= 2) {
            create_kernel_task("idle", idle);           /* Create the kernel idle loop */
        }
        /* Done doing our work, now just wait */
        preempt(false);
    }
}

void init_services(void) {
    create_kernel_task("idle", idle);           /* Create the kernel idle loop */
    create_user_task("awesome_user", awesomefunc); /* Create the test user task*/
}

void start_shell(void) {
    /* Do some testing for now */
    tarfs_entry e;
    if(traverse_tars("bin/hello", &e) != NULL) {
        printk("name: %s\nLocation: %p\nSize: %d\n", e.path, e.data_base, e.size);
        /* Try to load file as elf */
        load_elf(e.data_base, e.size, copy_page_tables(get_cr3()));
    } else {
        printk("Unable to find: %s in tarfs\n", "bin/hello");
    }
}
