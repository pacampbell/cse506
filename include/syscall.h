#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <sys/defs.h>
#include <sys/syscall.h>

/*
static __inline uint64_t syscall_0(uint64_t n) {
	return 0;
}

static __inline uint64_t syscall_1(uint64_t n, uint64_t a1) {
	uint64_t rval = 0;
	__asm__ volatile (
			"movq %0, %%rax;"
		    "movq %1, %%rbx;"
			"syscall;"
		    : "=a"(rval)
			:"a"(n), "b"(a1)
			:
		);
	return rval;
}

static __inline uint64_t syscall_2(uint64_t n, uint64_t a1, uint64_t a2) {
	return 0;
}

static __inline uint64_t syscall_3(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3) {
	uint64_t rval = 0;
	__asm__ volatile (
			"movq %0, %%rax;"
			"movq %1, %%rbx;"
			"movq %2, %%rbx;"
			"movq %3, %%rbx;"
			"syscall;"
			: "=a"(rval)
			:"a"(n), "b"(a1), "c"(a2), "d"(a3)
			:
		);
	return rval;
}
*/

// Should find a better place for this, but it will do for now.
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

/*
#define syscall_0(NUM)							\
({												\
	uint64_t rv = -ENOSYS;						\
	__asm__ volatile (							\
	"movq %1, %%rax;"                     		\
	"syscall;"                          		\
	"movq %%rax, %0;"                     		\
	:"=a"(rv)                             		\
	:"a"(NUM)                             		\
	:);				        					\
	rv;											\
})

#define syscall_1(NUM, ARG1)					\
({												\
	uint64_t rv = -ENOSYS;						\
	__asm__ volatile (                          \
	"movq %1, %%rax;"                           \
	"movq %2, %%rdi;"                           \
	"syscall;"									\
	"movq %%rax, %0;"							\
	:"=a"(rv)                                   \
	:"a"(NUM), "b"(ARG1)                        \
	:);				                			\
	rv;											\
})
*/

static __inline uint64_t syscall_1(uint64_t n, uint64_t a1) {
	uint64_t rv = -ENOSYS;
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

/*
#define syscall_2(NUM, ARG1, ARG2)				\
({												\
	uint64_t rv = -ENOSYS;						\
	__asm__ volatile (                          \
	"movq %1, %%rax;"                           \
	"movq %2, %%rdi;"                           \
	"movq %3, %%rsi;"                           \
	"syscall;"									\
	"movq %%rax, %0;"							\
	:"=a"(rv)                                   \
	:"a"(NUM), "b"(ARG1), "c"(ARG2)             \
	:);											\
	rv;											\
})


#define syscall_3(NUM, ARG1, ARG2, ARG3)		\
({												\
	uint64_t rv = -ENOSYS;						\
	__asm__ __volatile__ (                      \
	"movq %1, %%rax;"                           \
	"movq %2, %%rdi;"                           \
	"movq %3, %%rsi;"                           \
	"movq %4, %%rdx;"                           \
	"syscall;"									\
	"movq %%rax, %0;"							\
	:"=r"(rv)                                   \
	:"r"(NUM), "r"(ARG1), "r"(ARG2), "r"(ARG3)  \
	:"%rax", "%rdi", "%rsi", "%rdx");											\
	rv;											\
})
*/

static __inline uint64_t syscall_3(uint64_t n, uint64_t a1, uint64_t a2, uint64_t a3) {
	uint64_t rv = 0;
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

/*
#define syscall_4(NUM, ARG1, ARG2, ARG3, ARG4)		\
({													\
	uint64_t rv = -ENOSYS;							\
	__asm__ volatile (                              \
	"movq %1, %%rax;"                               \
	"movq %2, %%rdi;"                               \
	"movq %3, %%rsi;"                               \
	"movq %4, %%rdx;"                               \
	"movq %5, %%r10;"                               \
	"syscall;"										\
	"movq %%eax, %0;"								\
	:"=a"(rv)                                       \
	:"a"(NUM), "b"(ARG1), "c"(ARG2), "d"(ARG3), "s"(ARG4)\
	:);												\
	rv;												\
})

#define syscall_5(NUM, ARG1, ARG2, ARG3, ARG4, ARG5)	\
({														\
	uint64_t rv = -ENOSYS;								\
	__asm__ volatile (                                  \
	"movq %1, %%rax;"                               	\
	"movq %2, %%rdi;"                               	\
	"movq %3, %%rsi;"                               	\
	"movq %4, %%rdx;"                               	\
	"movq %5, %%r10;"                               	\
	"movq %6, %%r8;"                               	\
	"syscall;"											\
	"movq %%rax, %0;"									\
	:"=a"(rv)                                       	\
	:"a"(NUM), "b"(ARG1), "c"(ARG2), "d"(ARG3), "s"(ARG4), "D"(ARG5)\
	:);													\
	rv;													\
})
*/
/*
#define syscall_6(NUM, ARG1, ARG2, ARG3, ARG4, ARG5, ARG6) 		\
({							     								\
	uint64_t rv = -ENOSYS;					     				\
	__asm__ volatile (											\
	"movq %1, %%rax;"                     						\
	"push %7;"													\
	"push %6;"													\
	"push %5;"													\
	"push %4;"													\
	"push %3;"													\
	"push %2;"													\
	"movq %%rsp, %%rbx;"										\
	"syscall;"                          						\
	"movq %%rax, %0;"                     						\
	:"=a"(rv)                             						\
	:"a"(NUM), "rm"(ARG1), "rm"(ARG2), "rm"(ARG3), "rm"(ARG4), "rm"(ARG5), "rm"(ARG6)\
	:"%rbx");													\
	rv;						     								\
})
*/

#endif
