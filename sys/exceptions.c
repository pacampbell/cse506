#define __KERNEL__
#include <sys/exceptions.h>
#include <sys/task.h>


/* Private handler prototypes */
static void divide_by_zero_fault(registers_t regs);
static void nmi_interrupt(registers_t regs);
static void overflow_trap(registers_t regs);
static void bound_range_exceded(registers_t regs); 
static void invalid_opcode_fault(registers_t regs);
static void device_not_available_fault(registers_t regs);
static void double_fault_abort(registers_t regs);
static void invalid_tss_fault(registers_t regs);
static void segment_not_present_fault(registers_t regs);
static void stack_segmentation_fault(registers_t regs);
static void general_protection_fault(registers_t regs);
static void page_fault(registers_t regs);
static void alignment_check_fault(registers_t regs);
static void machine_check_abort(registers_t regs);

void initialize_fault_handlers(void) {
	register_interrupt_handler(EXCEPTION_DIVIDE_BY_ZERO, &divide_by_zero_fault);
	register_interrupt_handler(EXCEPTION_NMI, &nmi_interrupt);
	register_interrupt_handler(EXCEPTION_OVERFLOW, &overflow_trap);
	register_interrupt_handler(EXCEPTION_BOUNDS_EXCEEDED, &bound_range_exceded);
	register_interrupt_handler(EXCEPTION_INVALID_OPCODE, &invalid_opcode_fault);
	register_interrupt_handler(EXCEPTION_DEVICE, &device_not_available_fault);
	register_interrupt_handler(EXCEPTION_DOUBLE_FAULT, &double_fault_abort);
	register_interrupt_handler(EXCEPTION_INVALID_TSS, &invalid_tss_fault);
	register_interrupt_handler(EXCEPTION_SEGMENT_NP, &segment_not_present_fault);
	register_interrupt_handler(EXCEPTION_STACK_SEG_FLT, &stack_segmentation_fault);
	register_interrupt_handler(EXCEPTION_GPF, &general_protection_fault);
	register_interrupt_handler(EXCEPTION_PF, &page_fault);
	register_interrupt_handler(EXCEPTION_ALIGNMENT_CHECK, &alignment_check_fault);
	register_interrupt_handler(EXCEPTION_MACHINE_CHECK, &machine_check_abort);
}

static void divide_by_zero_fault(registers_t regs) {
	PANIC("DIVIDE BY ZERO\n");
}

static void nmi_interrupt(registers_t regs) {
	PANIC("NMI\n");
}

static void overflow_trap(registers_t regs) {
	PANIC("OVERFLOW\n");
}

static void bound_range_exceded(registers_t regs) {
	PANIC("BOUNDS EXCEDED\n");
}

static void invalid_opcode_fault(registers_t regs) {
	PANIC("INVALID OPCODE\n");
}

static void device_not_available_fault(registers_t regs) {
	PANIC("DEVICE NOT AVILABLE\n");
}

static void double_fault_abort(registers_t regs) {
	PANIC("DOUBLE FAULT\n");
}

static void invalid_tss_fault(registers_t regs){
	PANIC("INVALID TSS\n");
}

static void segment_not_present_fault(registers_t regs) {
	PANIC("SEGEMNT NOT PRESENT\n");
}

static void stack_segmentation_fault(registers_t regs) {
	PANIC("STACK SEGMENTATION\n");
}

static void general_protection_fault(registers_t regs) {
	panic("GENERAL PROTECTION FAULT\n");
	printk("Interrupt [%d] - Errocode: %p\n", regs.int_no, regs.err_code);
	printk("rip: %p ss: %p cs: %p ds: %p\n", regs.rip, regs.ss, regs.cs, regs.ds);
	panic("DUMP\n");
	Task *ctask = get_current_task();
	dump_task(ctask);
	__asm__ __volatile__("cli; hlt;");
}

static void page_fault(registers_t regs) {
	uint64_t faulting_address;
    __asm__ __volatile__("mov %%cr2, %0" : "=r" (faulting_address));

    panic("!!!!!!!PAGE FAULT!!!!!!!\n");
    printk("virt address: %p\n", faulting_address);
    printk("phys address: %p\n", VIRT_TO_PHYS(faulting_address));
    printk("bits   : ");

    int p = extract_bits(regs.err_code, 0, 0);
    int w = extract_bits(regs.err_code, 1, 1);
    int u = extract_bits(regs.err_code, 2, 2);
    int r = extract_bits(regs.err_code, 3, 3);
    int f = extract_bits(regs.err_code, 4, 4);
    int cow = extract_bits(regs.err_code, 63, 63);

    printk("%x", p);
    printk("%x", w);
    printk("%x", u);
    printk("%x", r);
    printk("%x", f);
    printk(" %x\n", cow);

    printk("Reason: [");

    if(cow) {
        printk("cow ");
    }

    if(p) {
        printk("prot"); 
    } else {
        printk("mia"); 
    }

    if(w) {
        printk(" write"); 
    } else {
        printk(" read"); 
    }
    
    if(u) {
        printk(" user"); 
    } else {
        printk(" su"); 
    }

    if(r) {
        printk(" reserved"); 
    }

    if(f) {
        printk("fetch]\n"); 
    } else {
        printk("]\n");
    }
    // Dump info about the task
    Task *ctask = get_current_task();
    dump_task(ctask);

    __asm__ __volatile__("cli;hlt;");
}

static void alignment_check_fault(registers_t regs) {
	PANIC("ALIGNMENT CHECK\n");
}

static void machine_check_abort(registers_t regs) {
	PANIC("MACHINE CHECK\n");
}
