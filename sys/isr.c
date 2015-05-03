#define __KERNEL__
#include <sys/isr.h>
#include <sys/common.h>

void printk(const char *fmt, ...);

isr_t interrupt_handlers[256];

void irq_handler(registers_t regs) {
    // Send an EOI (end of interrupt) signal to the PICs.
    // If this interrupt involved the slave.
    if (regs.int_no >= 40) {
        // Send reset signal to slave.
        outb(0xA0, 0x20);
    }
    // Send reset signal to master. (As well as slave, if necessary).
    outb(0x20, 0x20);

    if (interrupt_handlers[regs.int_no] != 0) {
        isr_t handler = interrupt_handlers[regs.int_no];
        handler(regs);
    }
}


void register_interrupt_handler(uint8_t n, isr_t handler) {
      interrupt_handlers[n] = handler;
}

/* FIXME: need for debugging; should remove this at some point .. */
void printk(const char *fmt, ...);

/*static*/ char* strintno(uint64_t int_no) {
    switch(int_no) {
        case HW_ISR_DIV_BY_ZERO:
            return "DIV_BY_ZERO";
        case HW_ISR_DEBUGGER:
            return "DEBUGGER";
        case HW_ISR_NMI:
            return "NMI";
        case HW_ISR_BREAKPOINT:
            return "BREAKPOINT";
        case HW_ISR_OVERFLOW:
            return "OVERFLOW";
        case HW_ISR_BOUNDS:
            return "BOUNDS";
        case HW_ISR_INVALID_OPCODE:
            return "INVALID_OPCODE";
        case HW_ISR_COPROCESSOR_NOT_AVAIL:
            return "NO_MATH_COPROCESSOR";
        case HW_ISR_DOUBLE_FAULT:
            return "DOUBLE_FAULT";
        case HW_ISR_COPROCESSOR_SEGMENT_OVERRUN:
            return "COPROCESSOR_SEGMENT_OVERRUN";
        case HW_ISR_INVALID_TASK_STATE_SEGMENT:
            return "INVALID_TASK_STATE_SEGMENT_OVERRUN";
        case HW_ISR_SEGMENT_NOT_PRESENT:
            return "SEGMENT_NOT_PRESENT";
        case HW_ISR_STACK_FAULT:
            return "STACK_SEGMENT_FAULT";
        case HW_ISR_GENERAL_PROTECTION_FAULT:
            return "GENERAL_PROTECTION_FAULT";
        case HW_ISR_PAGE_FAULT:
            return "PAGE_FAULT";
        case HW_ISR_RESERVED_15:
            return "RESERVED_15;UNKNOWN_INTERRUPT";
        case HW_ISR_MATH_FAULT:
            return "FLOATING_POINT_MATH_ERROR";
        case HW_ISR_ALIGNMENT_CHECK:
            return "ALIGNMENT_CHECK";
        case HW_ISR_MACHINE_CHECK:
            return "MACHINE_CHECK";
        case HW_ISR_SIMD_FP_EXCEPTION:
            return "SIMD_FLOATING_POINT_EXCEPTION";
        /* All reserved interrupts share same message */
        case 20:
        case 21:
        case 22:
        case 23:
        case 24:
        case 25:
        case 26:
        case 27:
        case 28:
        case 29:
        case 30:
        case 31:
            return "RESERVED_NOT_IN_USE";
        default:
            return "UNKNOWN_INTERRUPT_NO";
    }
}

void isr_handler(registers_t regs) {
    char *str_interrupt = strintno(regs.int_no);
    printk("recieved interrupt[%d]: %s\n", regs.int_no, str_interrupt);

    if (interrupt_handlers[regs.int_no] != 0) {
        isr_t handler = interrupt_handlers[regs.int_no];
        handler(regs);
    }
    __asm__ __volatile__("cli;hlt;");
}
