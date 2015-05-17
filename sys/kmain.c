#define __KERNEL__
#include <sbunix/kmain.h>
#include <sbunix/debug.h>

void idle(void) {
    while(1) {
        __asm__ __volatile__("hlt;");
        preempt(false, false);
    }
}

/**
 * Similar to initd, this is process 0. This starts up all kernel level 
 * services and threads. 
 */
void kmain(void) {
    // ls_tars();
    /* do some basic setup */
    cls();
    init_services();
    /* start the shell */
    start_shell();
    ls_tars("bin");
    printk("\n");
    /* Everything is started now spin */
    while(1) {
        /* Done doing our work, now just wait */
        preempt(false, false);
    }
}

void init_services(void) {
    /* WARNING */
    /* If you change the order you should update constants in <sys/task.h> */
    create_kernel_task("idle", idle);           /* Should be pid 1 */
    /* END WARNING */
}

void start_shell(void) {
  // int argc = 1;
  // char *argv[] = {"bin/sbush",NULL};
  // char *envp[] = {"PATH=bin/", NULL};

    // printk("sizeof(registers): %d\n", __builtin_offsetof(Task, kstack));
    // halt();
    /* Do some testing for now */
    // exec_tarfs_elf_args("bin/cat", argc, argv, envp);
    // exec_tarfs_elf_args("bin/sbush", argc, argv, envp);
    // exec_tarfs_elf_args("bin/args", argc, argv, envp);
    // exec_tarfs_elf_args("bin/exec", argc, argv, envp);
    // exec_tarfs_elf_args("bin/askexec", argc, argv, envp);
    // exec_tarfs_elf("bin/ps");
    // exec_tarfs_elf("bin/ps");
    // exec_tarfs_elf("bin/ps");
    // exec_tarfs_elf("bin/ps");
    // exec_tarfs_elf("bin/ps");
    // exec_tarfs_elf("bin/fork");
    // exec_tarfs_elf("bin/fork");
    // exec_tarfs_elf("bin/fork");
    // exec_tarfs_elf("bin/fork");
    // exec_tarfs_elf("bin/ps");
    // exec_tarfs_elf("bin/hello");
    exec_tarfs_elf("bin/fork");
    
    // exec_tarfs_elf("bin/ps");
    // for(int i = 0; i < 10; i++) {
    //     exec_tarfs_elf("bin/hello");
    // }
    // exec_tarfs_elf("bin/yield");
    // exec_tarfs_elf("bin/kill");
    // exec_tarfs_elf("bin/open");
    // exec_tarfs_elf("bin/malloc");
}
