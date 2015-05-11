#define __KERNEL__
#include <sys/timer.h>
#include <sys/isr.h>
#include <sys/screen.h>
#include <sbunix/string.h>
#include <sbunix/kernel.h>

volatile uint32_t tick = 0;
static char string_buffer[1024];

static void print_timer(void) {
    itoa((int)tick, string_buffer);
    int start_x = 79 - strlen(string_buffer);
    for(int i = 0; string_buffer[i] != '\0'; i++) {
        putk_xy(string_buffer[i], start_x + i, 24);
    }
}

static void timer_callback(registers_t regs) {
    print_timer();
    tick++;
}

void init_timer(uint32_t frequency) {
    // Firstly, register our timer callback.
    register_interrupt_handler(IRQ0, &timer_callback);

    // The value we send to the PIT is the value to divide it's input clock
    // (1193180 Hz) by, to get our required frequency. Important to note is
    // that the divisor must be small enough to fit into 16-bits.
    uint32_t divisor = 1193180 / frequency;

    // Send the command byte.
    outb(0x43, 0x36);

    // Divisor has to be sent byte-wise, so split here into upper/lower bytes.
    uint8_t l = (uint8_t)(divisor & 0xFF);
    uint8_t h = (uint8_t)( (divisor>>8) & 0xFF );

    // Send the frequency divisor.
    outb(0x40, l);
    outb(0x40, h);
}
