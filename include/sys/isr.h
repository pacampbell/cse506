#ifndef _ISR_H
#define _ISR_H
#include <sys/defs.h>
typedef struct registers
{
   uint64_t ds;                  // Data segment selector
   uint64_t rdi, rsi, rbp, rsp, rbx, rdx, rcx, rax; // Pushed by pusha.
   uint64_t int_no, err_code;    // Interrupt number and error code (if applicable)
   uint64_t rip, cs, eflags, useresp, ss; // Pushed by the processor automatically.
} registers_t;
#endif
