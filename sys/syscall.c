#define __KERNEL__
#include <sys/syscall.h>
#include <sys/syscall_k.h>
#include <sys/task.h>
#include <sys/screen.h>

void sys_exit(int ret) {
	// Unschedule the current task.
	preempt(true);
}

// int fd, const void *buf, size_t count
void sys_write(int fd, char *buff, size_t count) {
	// Always write the stdout and stderr to the same place
	if(fd == 1 || fd == 2) {
		for(int i = 0; i <= count; i++) {
			putk(buff[i]);
		}
	} else {
		panic("sys_write called for an unimplemented FD.");
	}
}

void sys_read(int fd, void *buff, size_t count) {

}

void sys_fork() {

}

void sys_exec() {

}

void sys_waitpid() {

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
void syscall_common_handler(void) {
	uint64_t num, arg1, arg2, arg3, arg4, arg5, arg6, ret, flags;
	__asm__ __volatile__(
		"movq %%rax, %0;"
		"movq %%rdi, %1;"
		"movq %%rsi, %2;"
		"movq %%rdx, %3;"
		"movq %%r10, %4;"
		"movq %%r8,  %5;"
		"movq %%r9,  %6;"
		/* Save the return address and flags */
		"movq %%rcx, %7;"
		"movq %%r11, %8;"
		: "=r"(num), "=r"(arg1), "=r"(arg2), "=r"(arg3), "=r"(arg4),
		  "=r"(arg5), "=r"(arg6), "=r"(ret), "=r"(flags)
		:
		:
	);
	
	switch(num) {
		case SYS_exit:
			sys_exit(arg1);
			break;
		case SYS_brk:
			panic("sys_brk not implemented.\n");
			break;
		case SYS_fork:
			panic("sys_fork not implemented.\n");
			break;
		case SYS_getpid:
			panic("sys_getpid not implemented.\n");
			break;
		case SYS_getppid:
			panic("sys_getppid not implemented.\n");
			break;
		case SYS_execve:
			panic("sys_execve not implemented.\n");
			break;
		case SYS_wait4:
			panic("sys_wait4 not implemented.\n");
			break;
		case SYS_nanosleep:
			panic("sys_nanosleep not implemented.\n");
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
			panic("sys_open not implemented.\n");
			break;
		case SYS_read:
			panic("sys_red not implemented.\n");
			break;
		case SYS_write:
			sys_write(arg1, (char*)arg2, arg3);
			break;
		case SYS_lseek:
			panic("sys_lseek not implemented.\n");
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
		default:
			printk("Unimplemented syscall %d\n", num);
			break;
	}
	__asm__ __volatile__(
		"movq %0, %%rcx;"
		"movq %1, %%r11;"
		"sysret;"
		:
		: "r"(ret), "r"(flags)
		:
		);
}

void write_msr(uint64_t msr, uint64_t lo, uint64_t hi) {
	// Upper 32 bits ignored on 64-bit registers
	// rax = low order
	// rdx = high order
	// rcx = msr
	__asm__ __volatile__(
		"movq %0, %%rcx;"
		"movq %1, %%rax;"
		"movq %2, %%rdx;"
		"wrmsr;"
		:
		: "r"(msr), "r"(lo), "r"(hi)
		: "rax", "rcx", "rdx"
	);
}

uint64_t read_msr(uint64_t msr) {
	// upper 32 bits ignored on 64-bit registers
	// rax = low order
	// rdx = high order
	// rcx = msr
	uint64_t lo = 0, hi = 0;
	__asm__ __volatile__(
		"movq %2, %%rcx;"
		"rdmsr;"
		"movq %%rax, %0;"  // lo
		"movq %%rdx, %1;"  // hi
		: "=r"(lo), "=r"(hi)
		: "r"(msr) 
		: "rcx"
	);
	return (hi << 32) | lo;
}

void init_syscall() {
	// check sce support
	uint64_t effer_val = read_msr(IA32_EFER); 
	// Set the SCE bit
	effer_val = effer_val | IA32_EFER_SCE;
	// Store the value back into the EFER
	write_msr(IA32_EFER, effer_val & 0xffffffff, (effer_val >> 32) & 0xffffffff);
	// Set the system call handler
	SET_LSTAR((uint64_t)syscall_common_handler);
	// Set the flags to clear
	uint64_t flag_mask = IA32_FLAGS_INTERRUPT | IA32_FLAGS_DIRECTION; 
	SET_FMASK(flag_mask);
}