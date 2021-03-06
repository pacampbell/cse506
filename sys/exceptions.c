#define __KERNEL__
#include <sys/exceptions.h>
#include <sys/task.h>
#include <sys/pgtable.h>
#include <sbunix/debug.h>


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
static void bad_page_fault(registers_t regs);
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
#ifdef DEBUG
    panic("GENERAL PROTECTION FAULT\n");
    printk("Interrupt [%d] - Errocode: %p\n", regs.int_no, regs.err_no);
    printk("rip: %p ss: %p cs: %p\n", regs.rip, regs.ss, regs.cs);
    panic("DUMP\n");
#endif
    Task *ctask = get_current_task();
    dump_task(ctask);
    printk("killing: %s\n", ctask->name);
    preempt(true, false);

}

static void page_fault(registers_t regs) {
    uint64_t faulting_address;
    __asm__ __volatile__("mov %%cr2, %0" : "=r" (faulting_address));
    Task *tsk = get_current_task();
#ifdef DEBUG
    printk("cr3: %p rip: %p faulting address: %p\n", get_cr3(), regs.rip, faulting_address);
    if(tsk->mm != NULL) {
        printk("brk: %p\n", tsk->mm->brk);
        printk("stack_start: %p\n", tsk->mm->start_stack);
    } else {
        printk("KERNEL TASK\n");
    }
#endif

    if(tsk->mm == NULL ||
            (faulting_address < tsk->mm->start_brk || tsk->mm->start_stack < faulting_address)) {
        bad_page_fault(regs);
    }

    uint64_t *address;
    if((address = kmalloc_vma((pml4_t*)tsk->registers.cr3, faulting_address & PG_ALIGN, 1, USER_SETTINGS)) == NULL) {
        panic("KMALLOC VMA FAILED\n");
    }

    struct vm_area_struct *vma = kmalloc_kern(sizeof(struct vm_area_struct));
    vma->vm_start = faulting_address & PG_ALIGN;
    vma->vm_end = vma->vm_start + PAGE_SIZE;
    vma->vm_prot = (tsk->mm->start_stack - faulting_address < faulting_address - tsk->mm->brk) ? VM_GROWSDOWN : 0;
    add_vma(tsk->mm, vma);


    // uint64_t page = get_pte((pml4_t*)(tsk->registers.cr3), (uint64_t)address);
    // printk("New Page: %p\n", page);
    // printk("Address returned: %p\n", address);
    check_vma_permissions((pml4_t*)(tsk->registers.cr3), (uint64_t)address);
    //halt();
    //panic("Good page fault\n");

}

static void bad_page_fault(registers_t regs) {
    uint64_t faulting_address;
    __asm__ __volatile__("mov %%cr2, %0" : "=r" (faulting_address));


    panic("!!!!!!!PAGE FAULT!!!!!!!\n");
#ifdef DEBUG
    printk("address: %p\n", faulting_address);
    printk("bits   : ");

    int p = extract_bits(regs.err_no, 0, 0);
    int w = extract_bits(regs.err_no, 1, 1);
    int u = extract_bits(regs.err_no, 2, 2);
    int r = extract_bits(regs.err_no, 3, 3);
    int f = extract_bits(regs.err_no, 4, 4);
    int cow = extract_bits(regs.err_no, 63, 63);

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

    printk("cs: %p ss: %p ursp: %p\n", regs.cs, regs.ss, regs.ursp);
#endif
    // Dump info about the task
    Task *ctask = get_current_task();
    dump_task(ctask);

    printk("killing: %s\n", ctask->name);
    preempt(true, false);
}

static void alignment_check_fault(registers_t regs) {
	PANIC("ALIGNMENT CHECK\n");
}

static void machine_check_abort(registers_t regs) {
	PANIC("MACHINE CHECK\n");
}
