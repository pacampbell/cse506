#ifndef __KERNEL__
#error "Trying to include kernel code in non-kernel code."
#else
    #ifndef __EXCEPTIONS_H
    #define __EXCEPTIONS_H
	#include <sys/sbunix.h>				// for Printk
	#include <sbunix/string.h>			// kernel string functions
	#include <sys/isr.h>                // For adding things to ISR
	

	#define EXCEPTION_DIVIDE_BY_ZERO 	0
	#define EXCEPTION_DEBUG 			1
	#define EXCEPTION_NMI 				2
	#define EXCEPTION_BREAKPOINT	    3
	#define EXCEPTION_OVERFLOW			4
	#define EXCEPTION_BOUNDS_EXCEEDED	5
	#define EXCEPTION_INVALID_OPCODE	6
	#define EXCEPTION_DEVICE 			7
	#define EXCEPTION_DOUBLE_FAULT		8
	#define EXCEPTION_COPROC_OVERRUN	9
	#define EXCEPTION_INVALID_TSS		10
	#define EXCEPTION_SEGMENT_NP	    11
	#define EXCEPTION_STACK_SEG_FLT		12
	#define EXCEPTION_GPF				13
	#define EXCEPTION_PF 				14
	#define EXCEPTION_X87_FP			16
	#define EXCEPTION_ALIGNMENT_CHECK	17
	#define EXCEPTION_MACHINE_CHECK 	18
	#define EXCEPTION_SIMD				19
	#define EXCEPTION_VIRT				20
	#define EXCEPTION_SECURITY			30
	// #define EXCEPTION_ Triple Fault	-	-	-	No
	// #define EXCEPTION_ FPU Error Interrupt	IRQ 13

	/* Helper macros */
	#define PANIC(value) do {               \
		panic((value));                     \
		__asm__ __volatile__("cli; hlt;");  \
    } while(0)

	/* Call for initialization of all fault handlers */
	void initialize_fault_handlers(void);	
	#endif
#endif
