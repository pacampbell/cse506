#define __KERNEL__
#include <sbunix/kmain.h>

void awesomefunc(void) {
    /*
    int a = 1000;
    while(a--) {

        preempt(false);
    }
    */
    char *str = "I'm awesome!!! woot woot";
    __asm__ __volatile__(
        "movq $1, %%rax;"
        "movq $1, %%rdi;"
        "movq %0, %%rsi;"
        "movq $24, %%rdx;"
        "syscall;"
        :
        : "r"(str)
        : "rax", "rdi", "rsi"
        );
    // yield
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
    start_shell();
    /* Everything is started now spin */
    while(1) {
        /* Done doing our work, now just wait */
        preempt(false);
    }
}

void init_services(void) {
    /* WARNING */
    /* If you change the order you should update constants in <sys/task.h> */
    create_kernel_task("idle", idle);           /* Should be pid 1 */
    /* END WARNING */
    // create_user_task("awesome_func", awesomefunc);
}

void start_shell(void) {
    /* Do some testing for now */
    tarfs_entry e;
    if(traverse_tars("bin/hello", &e) != NULL) {
        printk("name: %s\nLocation: %p\nSize: %d\n", e.path, e.data_base, e.size);
        /* Try to load file as elf */
        create_user_elf_task("test-user", e.data_base, e.size);
        //load_elf(e.data_base, e.size, copy_page_tables(get_cr3()));
    } else {
        printk("Unable to find: %s in tarfs\n", "bin/hello");
    }
}
