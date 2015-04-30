#define __KERNEL__
#include <sys/isr.h>
#include <sys/screen.h>
#include <sbunix/string.h>
#include <sbunix/kernel.h>
#include <sys/sbunix.h>

static void pg_fault_callback(registers_t regs) {
    uint64_t faulting_address;
    __asm__ __volatile__("mov %%cr2, %0" : "=r" (faulting_address));

    panic("!!!!!!!PAGE FAULT!!!!!!!\n");
    printk("address: %p\n", faulting_address);
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


    if(cow) {
        printk("The access requires copy on write which is NOT implemented.\n");
    }


    /*
    if(p) {
        printk("The fault was caused by a page-level protection violation.\n");
    } else {
        printk("The fault was caused by a non-present page.\n");
    }
    */
    if(w) {
        printk("The access causing the fault was a write.\n");
    } else {
        printk("The access causing the fault was a read.\n");
    }

    /*    
    if(u) {
        printk("A user-mode access caused the fault.\n");
    } else {
        printk("A supervisor-mode access caused the fault.\n");
    }

    if(r) {
        printk("The fault was caused by a reserved bit set to 1 in some paging-structure entry.\n");
    } else {
        printk("The fault was not caused by reserved bit violation.\n");
    }

    if(f) {
        printk("The fault was caused by an instruction fetch.\n");
    } else {
        printk("The fault was not caused by an instruction fetch.\n");
    }
    */

    __asm__ __volatile__("cli;hlt;");
}

void init_pg_fault() {
    register_interrupt_handler(14, &pg_fault_callback);
}
