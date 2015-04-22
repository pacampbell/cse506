#define __KERNEL__
#include <sys/timer.h>
#include <sys/isr.h>
#include <sys/screen.h>
#include <sbunix/string.h>
#include <sbunix/kernel.h>
#include <sys/sbunix.h>

static void pg_fault_callback() {
    printk("!!!!!!!!!!!!!!!!!!!!!!!\n");
    printk("!!!!!!PAGE FAULT!!!!!!!\n");
    printk("!!!!!!!!!!!!!!!!!!!!!!!\n");
}

void init_pg_fault() {
    // Firstly, register our pg fault callback.
    register_interrupt_handler(IRQ14, &pg_fault_callback);

}
