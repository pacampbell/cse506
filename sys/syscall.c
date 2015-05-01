#define __KERNEL__
#include <sys/syscall.h>
#include <sys/syscall_k.h>

extern void printk();

void sys_exit(int ret) {

}

void write(int fd, const char *buff, size_t count) {

}

void fork() {

}

void exec() {

}

void waitpid() {

}

void syscall_common_handler(void) {
	// when syscall is called the return addres is stored in RCX
	// return value should go in %%rax
	// flags into R11
	// %rax = syscall #
    // %rdi
    // %rsi
    // %rdx
    // %r10
    // %r8
    // %r9
	uint64_t num, arg1, arg2, arg3, arg4, arg5, arg6;
	__asm__ __volatile__(
		"movq %%rax, %0;"
		"movq %%rdi, %1;"
		"movq %%rsi, %2;"
		"movq %%rdx, %3;"
		"movq %%r10, %4;"
		"movq %%r8,  %5;"
		"movq %%r9,  %6;"
		: "=r"(num), "=r"(arg1), "=r"(arg2), "=r"(arg3), "=r"(arg4),
		  "=r"(arg5), "=r"(arg6)
		:
		:
	);
	printk("Syscall common called - NOT IMPLEMENTED\n");
	switch(num) {
		case SYS_exit:
			break;
		case SYS_brk:
			break;
		case SYS_fork:
			break;
		case SYS_getpid:
			break;
		case SYS_getppid:
			break;
		case SYS_execve:
			break;
		case SYS_wait4:
			break;
		case SYS_nanosleep:
			break;
		case SYS_alarm:
			break;
		case SYS_getcwd:
			break;
		case SYS_chdir:
			break;
		case SYS_open:
			break;
		case SYS_read:
			break;
		case SYS_write:
			break;
		case SYS_lseek:
			break;
		case SYS_close:
			break;
		case SYS_pipe:
			break;
		case SYS_dup:
			break;
		case SYS_dup2:
			break;
		case SYS_getdents:
			break;
		case SYS_mmap:
			break;
		case SYS_munmap:
			break;
		default:
			printk("Unimplemented syscall %d\n", num);
			break;
	}
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
}