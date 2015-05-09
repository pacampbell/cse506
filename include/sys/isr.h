#ifndef _ISR_H
#define _ISR_H

#include <sys/idt.h>
#include <sys/defs.h>

#define IRQ0  32
#define IRQ1  33
#define IRQ2  34
#define IRQ3  35
#define IRQ4  36
#define IRQ5  37
#define IRQ6  38
#define IRQ7  39
#define IRQ8  40
#define IRQ9  41
#define IRQ10 42
#define IRQ11 43
#define IRQ12 44
#define IRQ13 45
#define IRQ14 46
#define IRQ15 47

// Enables registration of callbacks for interrupts or IRQs.
// For IRQs, to ease confusion, use the #defines above as the
// first parameter.


// struct registers {
//     uint64_t r15, r14, r13, r12, r11, r10;
//     uint64_t r9, r8, rdi, rsi, rdx, rcx;
//     uint64_t rbx, rax, rbp;
//     uint64_t rip, cs, rflags, rsp, ss;      // Registers pushed by the IRETQ 
// } __attribute__((packed));




/*
struct registers {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8, rsi, rbp, rdx, rcx, rbx, rax, rdi; // Registers pushed by PUSHA
    uint64_t int_no, err_no;                            // Interrupt number and error code
    uint64_t rip, cs, rflags, rsp, ss;                  // Registers pushed by the IRETQ 
} __attribute__((packed));
typedef struct registers registers_t;
*/


struct registers {
   uint64_t ds;                  // Data segment selector
   uint64_t r15, r14, r13, r12, r11, r10, r9, r8, rdi, rsi, rbp, rdx, rcx, rbx, rax;
   // uint64_t rdi, rsi, rbp, rbx, rdx, rcx, rax; // Pushed by pusha.
   uint64_t int_no, err_no;    // Interrupt number and error code (if applicable)
   uint64_t rip, cs, rflags, ursp, ss; // Pushed by the processor automatically.
} __attribute__((packed));
typedef struct registers registers_t;

typedef void (*isr_t)(registers_t);
void register_interrupt_handler(uint8_t n, isr_t handler);

#endif
