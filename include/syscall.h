#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <sys/defs.h>
#include <sys/syscall.h>

// FIXME: Should find a better place for this, but it will do for now.
#define ENOSYS          38      				/* Function not implemented */

/**
 Syscall table - different on x86 vs x86_64
 http://blog.rchapman.org/post/36801038863/linux-system-call-table-for-x86-64

 %rax = syscall #
 %rdi
 %rsi
 %rdx
 %r10
 %r8
 %r9

 If anything is returned, the result is in %rax
 When using x86_64, with 64bit registers (ex: rax), we need to push, move, etc
 using the q modifier.

 The syntax used by linux assembly and gas (GNU assembler) is at&t which is opposite
 of intel, mips, etc.

 movq s, d   # Moves the value s into d
*/

static __inline int64_t syscall_0(uint64_t n) {
	int64_t rv = -ENOSYS;
	__asm__ __volatile__ (
		"movq %1, %%rax;"
		"syscall;"
		"movq %%rax, %0;"
		:"=r"(rv)
		:"r"(n)
		:"%rax"
	);
	return rv;
}

static __inline int64_t syscall_1(uint64_t n, uint64_t a1) {
	int64_t rv = -ENOSYS;
	__asm__ __volatile__ (
		"movq %1, %%rax;"
		"movq %2, %%rdi;"
		"syscall;"
		"movq %%rax, %0;"
		:"=r"(rv)
		:"r"(n), "r"(a1)
		:"%rax", "%rdi"
	);
	return rv;
}

static __inline int64_t syscall_2(uint64_t n, uint64_t a1, uint64_t a2) {
	int64_t rv = -ENOSYS;
	__asm__ __volatile__ (
	"movq %1, %%rax;"
	"movq %2, %%rdi;"
	"movq %3, %%rsi;"
	"syscall;"
	"movq %%rax, %0;"
	:"=r"(rv)
	:"r"(n), "r"(a1), "r"(a2)
	:"%rax", "%rdi", "%rsi");
	return rv;
}

static __inline int64_t syscall_3(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3) {
	int64_t rv = -ENOSYS;
	__asm__ __volatile__ (
		"movq %1, %%rax;"
		"movq %2, %%rdi;"
		"movq %3, %%rsi;"
		"movq %4, %%rdx;"
		"syscall;"
		"movq %%rax, %0;"
		:"=r"(rv)
		:"r"(n), "r"(a1), "r"(a2), "r"(a3)
		:"%rax", "%rdi", "%rsi", "%rdx"
	);
	return rv;
}

static __inline int64_t syscall_4(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
	int64_t rv = -ENOSYS;
	__asm__ __volatile__ (
		"movq %1, %%rax;"
		"movq %2, %%rdi;"
		"movq %3, %%rsi;"
		"movq %4, %%rdx;"
		"movq %5, %%r10;"
		"syscall;"
		"movq %%rax, %0;"
		:"=r"(rv)
		:"r"(n), "r"(a1), "r"(a2), "r"(a3), "r"(a4)
		:"%rax", "%rdi", "%rsi", "%rdx", "%r10"
	);
	return rv;
}

static __inline int64_t syscall_5(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5) {
	int64_t rv = -ENOSYS;
	__asm__ __volatile__ (
	"movq %1, %%rax;"
	"movq %2, %%rdi;"
	"movq %3, %%rsi;"
	"movq %4, %%rdx;"
	"movq %5, %%r10;"
	"movq %6, %%r8;"
	"syscall;"
	"movq %%rax, %0;"
	:"=r"(rv)
	:"r"(n), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5)
	:"%rax", "%rdi", "%rsi", "%rdx", "r10", "%r8");
	return rv;
}

static __inline int64_t syscall_6(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4, uint64_t a5, uint64_t a6) {
	int64_t rv = -ENOSYS;
	__asm__ __volatile__ (
	"movq %1, %%rax;"
	"movq %2, %%rdi;"
	"movq %3, %%rsi;"
	"movq %4, %%rdx;"
	"movq %5, %%r10;"
	"movq %6, %%r8;"
	"movq %7, %%r9;"
	"syscall;"
	"movq %%rax, %0;"
	:"=r"(rv)
	:"r"(n), "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(a6)
	:"%rax", "%rdi", "%rsi", "%rdx", "r10", "%r8", "%r9");
	return rv;
}

#endif
