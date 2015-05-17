#ifndef __KERNEL__
#error "Trying to include kernel code in non-kernel code."
#else
	#ifndef __SYSCALL_K_H
	#define __SYSCALL_K_H
		#include <sys/defs.h>
		#include <sys/syscall.h>
		#include <sys/time.h>
		#include <sys/task.h>
		#include <sys/screen.h>
		#include <sys/keyboard.h>
		// x86_64 MSRs.
		#define IA32_EFER						0xC0000080
		#define IA32_MSR_STAR					0xC0000081
		#define IA32_MSR_LSTAR					0xc0000082
		#define IA32_MSR_CSTAR                  0xc0000083
		#define IA32_MSR_FMASK					0xc0000084
		#define IA32_MSR_FS_BASE				0xc0000100
		#define IA32_MSR_GS_BASE				0xc0000101
		#define IA32_MSR_KERNEL_GS_BASE			0xc0000102

		// EFER Masks
		#define IA32_EFER_SCE 					0x00000001

		#define IA32_FLAGS_INTERRUPT			0x00000200
		#define IA32_FLAGS_DIRECTION			0x00000400

		/* System call initialzation and handler */
		uint64_t syscall_common_handler(uint64_t num, uint64_t arg1, uint64_t arg2, 
								    uint64_t arg3, uint64_t arg4, uint64_t arg5);
		void init_syscall(void);

		/* Functions for reading and writing to msfr*/
		void write_msr(uint64_t msr, uint64_t lo, uint64_t hi);
		uint64_t read_msr(uint64_t msr);

		/* syscall prototypes */
		void sys_yield();

		/* Macros for helping to set and check the MSR */
		#define IS_SCE_SET(value) ({ 			      \
			uint64_t val = (value) & IA32_EFER_SCE;   \
			val; 									  \
		})

		#define SET_EFER(value) do { \
		    write_msr(IA32_EFER, (value) & 0xffffffff, \
		    ((value) >> 32) & 0xffffffff); \
		} while(0)

		#define SET_LSTAR(value) do { \
			write_msr(IA32_MSR_LSTAR, (value) & 0xffffffff, \
		    ((value) >> 32) & 0xffffffff); \
		} while(0)

		#define SET_CSTAR(value) do { \
			write_msr(IA32_MSR_CSTAR, (value) & 0xffffffff, \
		    ((value) >> 32) & 0xffffffff); \
		} while(0)

		#define SET_STAR(value) do { \
			write_msr(IA32_MSR_STAR, (value) & 0xffffffff, \
		    ((value) >> 32) & 0xffffffff); \
		} while(0)

		#define SET_FMASK(value) do { \
			write_msr(IA32_MSR_FMASK, (value) & 0xffffffff, \
		    ((value) >> 32) & 0xffffffff); \
		} while(0)
	#endif
#endif