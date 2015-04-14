#include <sys/idt.h>


idt_entry_t idt_entries[MAX_IDT_ENTRIES];
idt_ptr_t idt_ptr;

/* FIXME: need for debugging; should remove this at some point .. */
void printk(const char *fmt, ...);

/* Function defined in idt.s */
extern void idt_flush(uint64_t idt_address);
extern void remap_pic(void);

/* Adds functions to the idt */
static void idt_set_gate(uint8_t number, uint64_t base, uint16_t selector, uint8_t flags);

/* initializes the idt */
void init_idt(void) {
    idt_ptr.limit = sizeof(idt_entry_t) * MAX_IDT_ENTRIES - 1;
    idt_ptr.base  = (uint64_t)&idt_entries;
    /* zero out the memory */
    memset(&idt_entries, 0, sizeof(idt_entry_t) * MAX_IDT_ENTRIES);
    /* Set interrupt vectors for the processor defined interrupts */
    idt_set_gate(HW_ISR_DIV_BY_ZERO, (uint64_t)isr_0, 0x08, 0x8E);
    idt_set_gate(HW_ISR_DEBUGGER, (uint64_t)isr_1, 0x08, 0x8E);
    idt_set_gate(HW_ISR_NMI, (uint64_t)isr_2, 0x08, 0x8E);
    idt_set_gate(HW_ISR_BREAKPOINT, (uint64_t)isr_3, 0x08, 0x8E);
    idt_set_gate(HW_ISR_OVERFLOW, (uint64_t)isr_4, 0x08, 0x8E);
    idt_set_gate(HW_ISR_BOUNDS, (uint64_t)isr_5, 0x08, 0x8E);
    idt_set_gate(HW_ISR_INVALID_OPCODE, (uint64_t)isr_6, 0x08, 0x8E);
    idt_set_gate(HW_ISR_COPROCESSOR_NOT_AVAIL, (uint64_t)isr_7, 0x08, 0x8E);
    idt_set_gate(HW_ISR_DOUBLE_FAULT, (uint64_t)isr_8, 0x08, 0x8E);
    idt_set_gate(HW_ISR_COPROCESSOR_SEGMENT_OVERRUN, (uint64_t)isr_9, 0x08, 0x8E);
    idt_set_gate(HW_ISR_INVALID_TASK_STATE_SEGMENT, (uint64_t)isr_10, 0x08, 0x8E);
    idt_set_gate(HW_ISR_SEGMENT_NOT_PRESENT, (uint64_t)isr_11, 0x08, 0x8E);
    idt_set_gate(HW_ISR_STACK_FAULT, (uint64_t)isr_12, 0x08, 0x8E);
    idt_set_gate(HW_ISR_GENERAL_PROTECTION_FAULT, (uint64_t)isr_13, 0x08, 0x8E);
    idt_set_gate(HW_ISR_PAGE_FAULT, (uint64_t)isr_14, 0x08, 0x8E);
    idt_set_gate(HW_ISR_RESERVED_15, (uint64_t)isr_15, 0x08, 0x8E);
    idt_set_gate(HW_ISR_MATH_FAULT, (uint64_t)isr_16, 0x08, 0x8E);
    idt_set_gate(HW_ISR_ALIGNMENT_CHECK, (uint64_t)isr_17, 0x08, 0x8E);
    idt_set_gate(HW_ISR_MACHINE_CHECK, (uint64_t)isr_18, 0x08, 0x8E);
    idt_set_gate(HW_ISR_SIMD_FP_EXCEPTION, (uint64_t)isr_19, 0x08, 0x8E);
    /* Interrupts [20,31] are reserved but not used */
    idt_set_gate(20, (uint64_t)isr_20, 0x08, 0x8E);
    idt_set_gate(21, (uint64_t)isr_21, 0x08, 0x8E);
    idt_set_gate(22, (uint64_t)isr_22, 0x08, 0x8E);
    idt_set_gate(23, (uint64_t)isr_23, 0x08, 0x8E);
    idt_set_gate(24, (uint64_t)isr_24, 0x08, 0x8E);
    idt_set_gate(25, (uint64_t)isr_25, 0x08, 0x8E);
    idt_set_gate(26, (uint64_t)isr_26, 0x08, 0x8E);
    idt_set_gate(27, (uint64_t)isr_27, 0x08, 0x8E);
    idt_set_gate(28, (uint64_t)isr_28, 0x08, 0x8E);
    idt_set_gate(29, (uint64_t)isr_29, 0x08, 0x8E);
    idt_set_gate(30, (uint64_t)isr_30, 0x08, 0x8E);
    idt_set_gate(31, (uint64_t)isr_31, 0x08, 0x8E);
    /* Set up the PIC IRQs */
    remap_pic();

    idt_set_gate(32, (uint64_t)irq_0, 0x08, 0x8E);
    idt_set_gate(33, (uint64_t)irq_1, 0x08, 0x8E);
    idt_set_gate(34, (uint64_t)irq_2, 0x08, 0x8E);
    idt_set_gate(35, (uint64_t)irq_3, 0x08, 0x8E);
    idt_set_gate(36, (uint64_t)irq_4, 0x08, 0x8E);
    idt_set_gate(37, (uint64_t)irq_5, 0x08, 0x8E);
    idt_set_gate(38, (uint64_t)irq_6, 0x08, 0x8E);
    idt_set_gate(39, (uint64_t)irq_7, 0x08, 0x8E);
    idt_set_gate(40, (uint64_t)irq_8, 0x08, 0x8E);
    idt_set_gate(41, (uint64_t)irq_9, 0x08, 0x8E);
    idt_set_gate(42, (uint64_t)irq_10, 0x08, 0x8E);
    idt_set_gate(43, (uint64_t)irq_11, 0x08, 0x8E);
    idt_set_gate(44, (uint64_t)irq_12, 0x08, 0x8E);
    idt_set_gate(45, (uint64_t)irq_13, 0x08, 0x8E);
    idt_set_gate(46, (uint64_t)irq_14, 0x08, 0x8E);
    idt_set_gate(47, (uint64_t)irq_15, 0x08, 0x8E);

    //printk("PIC IRQS set\n");

    /* TODO: Set kernel defined interrupts here */

    // Flush the idt
    idt_flush((uint64_t)&idt_ptr);
}

static void idt_set_gate(uint8_t number, uint64_t base, uint16_t selector, uint8_t flags) {
    // Cut up the first 32 bits into the two 16-bit variables
    idt_entries[number].base_low = base & 0xFFFF;
    idt_entries[number].base_mid = (base >> 16) & 0xFFFF;
    // Get the remaining 32 bits and store them into the high part of the base
    idt_entries[number].base_high = (base >> 32) & 0xFFFFFFFF;
    /* Set selectors and flags */
    idt_entries[number].selector = selector;
    idt_entries[number].flags = flags;
    /* Zero out reserved areas */
    idt_entries[number].reserved_ist = 0;
    idt_entries[number].reserved = 0;
}
