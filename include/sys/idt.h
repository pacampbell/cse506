#ifndef _IDT_H
#define _IDT_H
#define __KERNEL__
#include <sys/defs.h>
#include <sbunix/string.h>
/* 0-31 are reserved for the processor */
#define MAX_IDT_ENTRIES 256

/* structs and information found @ http://www.jamesmolloy.co.uk/tutorial_html/4.-The%20GDT%20and%20IDT.html */

// A struct describing an interrupt gate.
/**
 * Flags byte format - uint8_t flags
 * +-------------+------------------+-----------------------------------------+
 * | Bits [x,y)  |      Name        |       Description                       |
 * +-------------+------------------+-----------------------------------------+
 * | [0,5)       |                  | First five bits are ALWAYS 0b01110      |
 * +-------------+------------------+-----------------------------------------+
 * | [5,7)       |   DPL            | Privilege level from [0,3]              |
 * +-------------+------------------+-----------------------------------------+
 * | [7,8)       |   P              | Entry is present. If this is set to     |
 * |             |                  | zero "Interrupt not handeled" exception |
 * |             |                  | will occur.                             |
 * +-------------+------------------+-----------------------------------------+
 */

struct idt_entry_struct
{
   uint16_t base_low;                 // The lower 16 bits of the address to jump to when this interrupt fires.
   uint16_t selector;                 // Kernel segment selector.
   uint8_t  reserved_ist;             // This must always be zero.
   uint8_t  flags;                    // More flags. See documentation.
   uint16_t base_mid;
   uint32_t base_high;
   uint32_t reserved;
} __attribute__((packed));
typedef struct idt_entry_struct idt_entry_t;

// A struct describing a pointer to an array of interrupt handlers.
// This is in a format suitable for giving to 'lidt'.
struct idt_ptr_struct
{
   uint16_t limit;
   uint64_t base;                // The address of the first element in our idt_entry_t array.
} __attribute__((packed));
typedef struct idt_ptr_struct idt_ptr_t;

void init_idt(void);

/* externs for processor isr's 0-31 */
extern void isr_0();
extern void isr_1();
extern void isr_2();
extern void isr_3();
extern void isr_4();
extern void isr_5();
extern void isr_6();
extern void isr_7();
extern void isr_8();
extern void isr_9();
extern void isr_10();
extern void isr_11();
extern void isr_12();
extern void isr_13();
extern void isr_14();
extern void isr_15();
extern void isr_16();
extern void isr_17();
extern void isr_18();
extern void isr_19();
extern void isr_20();
extern void isr_21();
extern void isr_22();
extern void isr_23();
extern void isr_24();
extern void isr_25();
extern void isr_26();
extern void isr_27();
extern void isr_28();
extern void isr_29();
extern void isr_30();
extern void isr_31();

/* Constants for HW interrupt names [0,19] */
#define HW_ISR_DIV_BY_ZERO 0x00
#define HW_ISR_DEBUGGER 0x01
#define HW_ISR_NMI 0x02
#define HW_ISR_BREAKPOINT 0x03
#define HW_ISR_OVERFLOW 0x04
#define HW_ISR_BOUNDS 0x05
#define HW_ISR_INVALID_OPCODE 0x06
#define HW_ISR_COPROCESSOR_NOT_AVAIL 0x07
#define HW_ISR_DOUBLE_FAULT 0x08
#define HW_ISR_COPROCESSOR_SEGMENT_OVERRUN 0x09
#define HW_ISR_INVALID_TASK_STATE_SEGMENT 0x0A
#define HW_ISR_SEGMENT_NOT_PRESENT 0x0B
#define HW_ISR_STACK_FAULT 0x0C
#define HW_ISR_GENERAL_PROTECTION_FAULT 0x0D
#define HW_ISR_PAGE_FAULT 0x0E
#define HW_ISR_RESERVED_15 0x0F     /* Some places call this unknwon interrupt */
#define HW_ISR_MATH_FAULT 0x10
#define HW_ISR_ALIGNMENT_CHECK 0x11
#define HW_ISR_MACHINE_CHECK 0x12
#define HW_ISR_SIMD_FP_EXCEPTION 0x13
/* 20-31 are reserved but not used */


/* Extern for sbunix isr's */
// extern void

#endif
