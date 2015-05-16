#define __KERNEL__
#include <sys/syscall_k.h>
#include <sys/task.h>
#include <sys/pgtable.h>
#include <sbunix/debug.h>
#include <sys/tarfs.h>
#include <sys/elf.h>
#include <sys/fs/file.h>


uint64_t global_rip = 0;
uint64_t global_sp = 0;

void sys_ps();

void sys_exit(int ret) {
    // Unschedule the current task.
    preempt(true);
}

void sys_yield() {
    Task *task = get_current_task();
    if(task != NULL) {
        task->state = WAITING;
        preempt(false);
    } else {
        printk("NULL TASK\n");
    }
}

// int fd, const void *buf, size_t count
int sys_write(int fd, char *buff, size_t count) {
    // Always write the stdout and stderr to the same place
    if(fd == 1 || fd == 2) {
        for(int i = 0; i < count; i++) {
            putk(buff[i]);
        }
        return count;
    } else {
        panic("sys_write called for an unimplemented FD.");
        return 0;
    }
}

uint64_t sys_read(int fd, void *buff, size_t count) {
    uint64_t read = 0;
    if(fd == 1 || fd == 2 || fd < 0) {
        panic("sys_read called for an unimplemented FD.");
        return 0;
    }

    if(fd > 2) {
        Task *tsk = get_current_task();
        for(int i = 0; i < count && tsk->files[fd]->at < tsk->files[fd]->end; i++) {
            ((char*)buff)[i] = *((char*)tsk->files[fd]->at);
            tsk->files[fd]->at = tsk->files[fd]->at + 1;
            read++;
        }
        ((char*)buff)[read] = '\0';
    } else if (fd == 0) {
        read = gets((uint64_t)buff, count);
    }
    
    // gets((uint64_t)buff, count);
    return read;
}

off_t sys_lseek(int fd, off_t offset, int whence) {
        if (fd < 0 || fd > MAX_FD) return -1;

        Task *tsk = get_current_task();
        if(tsk->files[fd] == NULL) return -1;

        struct file *f = tsk->files[fd]; 

    switch (whence) {
        case SEEK_SET:
            f->at = f->start + offset;
            return offset;
        case SEEK_CUR:
            f->at += offset;
            return f->at - f->start;
        case SEEK_END:
            f->at = f->end + offset; 
            return f->at - f->start;
        default:
            panic("that wence not yet implemented");
            break;
    }
    return -1;
}

void* sys_brk(uint64_t addr) {
    Task *tsk = get_current_task();

    if (addr == 0) {
        return (void*)tsk->mm->brk;
    }

    if (addr >= tsk->mm->start_stack || tsk->mm->start_brk >= addr) {
        panic("ERROR: ");
        printk("adder: %p, brk: %p, stack: %p\n", addr, tsk->mm->brk, tsk->mm->start_stack);
        return NULL;
    }
    tsk->mm->brk = addr;

    return (void*)tsk->mm->brk;
    
}

uint64_t sys_fork() {
    // #1 Get current Task
    Task *current = get_current_task();
    // #2 clone task
    // printk("Cloning pid: %d name: %s\n", current->pid, current->name);
    Task *child = clone_task(current, global_sp, global_rip);
    // printk("Cloning of parent complete!\n");
    if(child != NULL) {
        // #3 schedule the task
        if(!insert_into_list(child)) {
            panic("FAILED TO INSERT CHILD PROCESS\n");
        }
    } else {
        panic("Failed to fork\n");
        return -1;
    }
    if(child->pid == current->pid) {
        // #4a we are in the child!!!
        return 0;
    } else {
        // #4b return new task pid
        return child->pid;
    }
}

void sys_waitpid() {

}

uint64_t sys_getpid() {
    Task *ctask = get_current_task();
    return ctask->pid;
}

uint64_t sys_getppid() {
    Task *ctask = get_current_task();
    uint64_t ppid = -1;
    if(ctask->parent == NULL) {
        ppid = KMAIN_PID;
    } else {
        ppid = ctask->parent->pid;
    }
    return ppid;
}

void sys_ps() {
    Task *task = get_task_list();
    printk("PID          TYPE            STATE            CMD\n");
    while(task != NULL) {
        if(task->state != TERMINATED) {
            printk("%d            %s          %s            %s\n",
                task->pid,
                task->type == KERNEL ? "KERNEL" : "USER  ",
                task->state == NEW ? "NEW" : task->state == READY ? "READY" : task->state == RUNNING ? "RUNNING" : task->state == WAITING ? "WAITING" : task->state == TERMINATED ? "TERMINATED" : "UNKNOWN",
                task->name);
        }
        task = task->next;
    }
}

extern uint32_t tick;
void sys_nanosleep(struct timespec *req, struct timespec *rem) {
    extern uint32_t tick;
    Task *task = get_current_task();
    if(task->sleep == -1) {
        task->sleep = tick + 100;
        task->state = WAITING;
    } 
    // yield until its time to wake up
    while(task->sleep > tick) {
        // Not ready to wake up just sleep
        printk("Sleeping until %d - currently: %d\n", task->sleep, tick);
        // BOCHS_MAGIC();
        preempt(false);
    }
    BOCHS_MAGIC();
    // Task is no longer sleeping. Reset
    task->sleep = -1;
    task->state = RUNNING;
}

int sys_open(const char *pathname, int flags) {
    Task *tsk = get_current_task();
    int rtn = -1;

    for (rtn = 3; rtn < MAX_FD && tsk->files[rtn] != NULL; rtn++);
    if (rtn >= MAX_FD) return -1;

    struct file *f = tarfs_to_file(pathname);
    if (f == NULL) return -1;

    tsk->files[rtn] = f;

    return rtn;
}

int sys_execve(char *filename, char *argv[], char *envp[]) {
    int argc;
    for(argc = 0; argv[argc] != NULL; argc++);
    exec_tarfs_elf_args(filename, argc, argv, envp);
    //exec_tarfs_elf(filename);
    preempt(true);

    return -1; 
}

int sys_kill(pid_t pid) {
    Task *tsk = get_task_by_pid(pid);
    tsk->state = TERMINATED;
    return 1;
}

/**
 * Upon entry to this function, all interrupts are disabeled, and we still are
 * using the stack of the userspace process.
 * The return address is stored in the rcx register.
 * the current flags are in the %r11 register.
 * The syscall and arguments are placed in following registers:
 * %%rax 		SYSCALL NUMBER
 * %%rdi		ARG1
 * %%rsi        ARG2
 * %%rdx        ARG3
 * %%r10        ARG4
 * %%r8         ARG5
 * %%r9         ARG6
 * 
 * A system call with more than 6 arguments will have its parameters on the 
 * stack.
 *
 *
 * Upon entry as stated above in this handler we switch to ring0 but are 
 * are currently using the userspace stack. We need to switch to the kernels
 * stack, and do what we need to do.
 *
 * Before calling sysret we need to do a few things:
 *   1) Restore the values of %%rcx and %%r11 because they are scratch registers
 *      so they make or may not of been overwritten. 
 *
 *   2) Switch the stack back to the userland stack. (%%rip)
 *
 *   3) <undetermined if this is neccessary> Set back %%rbp and GS.
http://www.vupen.com/blog/20120806.Advanced_Exploitation_of_Windows_Kernel_x64_Sysret_EoP_MS12-042_CVE-2012-0217.php
 *
 */
uint64_t syscall_common_handler(uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5) {
    uint64_t return_value = 0;
    switch(num) {
        case SYS_exit:
            sys_exit(arg1);
            break;
        case SYS_brk:
            return_value = (uint64_t)sys_brk(arg1);
            break;
        case SYS_fork:
            return_value = sys_fork();
            break;
        case SYS_getpid:
            return_value = sys_getpid();
            break;
        case SYS_getppid:
            return_value = sys_getppid();
            break;
        case SYS_execve:
            return_value = sys_execve((char *)arg1, (char **)arg2, (char **)arg3);
            break;
        case SYS_wait4:
            panic("sys_wait4 not implemented.\n");
            break;
        case SYS_nanosleep:
            sys_nanosleep((struct timespec*)arg1, (struct timespec*)arg2);
            break;
        case SYS_alarm:
            panic("sys_alarm not implemented.\n");
            break;
        case SYS_getcwd:
            panic("sys_getcwd not implemented.\n");
            break;
        case SYS_chdir:
            panic("sys_chdir not implemented.\n");
            break;
        case SYS_open:
            //panic("sys_open not implemented.\n");
            return_value = sys_open((const char*)arg1, arg2);
            break;
        case SYS_read:
            return_value = sys_read(arg1, (void*)arg2, arg3);
            break;
        case SYS_write:
            return_value = sys_write(arg1, (char*)arg2, arg3);
            break;
        case SYS_lseek:
            return_value = sys_lseek(arg1,arg2, arg3);
            break;
        case SYS_close:
            panic("sys_close not implemented.\n");
            break;
        case SYS_pipe:
            panic("sys_pipe not implemented.\n");
            break;
        case SYS_dup:
            panic("sys_dup not implemented.\n");
            break;
        case SYS_dup2:
            panic("sys_dup2 not implemented.\n");
            break;
        case SYS_getdents:
            panic("sys_getdents not implemented.\n");
            break;
        case SYS_mmap:
            panic("sys_mmap not implemented.\n");
            break;
        case SYS_munmap:
            panic("sys_munmap not implemented.\n");
            break;
        case SYS_ps:
            sys_ps();
            break;
        case SYS_yield:
            sys_yield();
            break;
        case SYS_kill:
            return_value = sys_kill(arg1);
            break;
        default:
            printk("Unimplemented syscall %d\n", num);
            break;
    }
    return return_value;
}

void write_msr(uint64_t msr, uint64_t lo, uint64_t hi) {
    // Upper 32 bits ignored on 64-bit registers
    // rax = low order
    // rdx = high order
    // rcx = msr
    __asm__ __volatile__(
            "wrmsr;"
            :
            : "c"(msr), "a"(lo), "d"(hi)
            :
            );
}

uint64_t read_msr(uint64_t msr) {
    // upper 32 bits ignored on 64-bit registers
    // rax = low order
    // rdx = high order
    // rcx = msr
    uint64_t lo = 0, hi = 0;
    __asm__ __volatile__(
            "rdmsr;"
            : "=a"(lo), "=d"(hi)
            : "c"(msr) 
            :
            );
    return (hi << 32) | lo;
}


extern void syscall_entry(void);
void init_syscall() {
    // check sce support
    uint64_t effer_val = read_msr(IA32_EFER); 
    // Set the SCE bit
    effer_val = effer_val | IA32_EFER_SCE;
    // Store the value back into the EFER
    write_msr(IA32_EFER, effer_val & 0xffffffff, (effer_val >> 32) & 0xffffffff);
    // Set the system call handler
    SET_LSTAR((uint64_t)syscall_entry);
    // Set STAR
    // uint64_t star_value = 0x1b23081000000000;
    uint64_t star_value = 0x001b000800000000;
    write_msr(IA32_MSR_STAR, star_value & 0xffffffff, (star_value >> 32) & 0xffffffff);
    // Set the flags to clear
    uint64_t flag_mask = IA32_FLAGS_INTERRUPT | IA32_FLAGS_DIRECTION; 
    SET_FMASK(flag_mask);
}
