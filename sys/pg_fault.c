#define __KERNEL__
#include <sys/timer.h>
#include <sys/isr.h>
#include <sys/screen.h>
#include <sbunix/string.h>
#include <sbunix/kernel.h>

static uint32_t tick = 0;
static char string_buffer[1024];

static void print_timer(void) {
    itoa((int)tick, string_buffer);
    int start_x = 79 - strlen(string_buffer);
    for(int i = 0; string_buffer[i] != '\0'; i++) {
        putk_xy(string_buffer[i], start_x + i, 24);
    }
}

static void pg_fault_callback(registers_t regs) {
    print_timer();
    tick++;
}

void init_pg_fault(uint32_t frequency) {
    // Firstly, register our pg fault callback.
    register_interrupt_handler(IRQ14, &pg_fault_callback);

}
