#include <sys/isr.h>
#include <sys/keyboard.h>

void printk(const char *format, ...);


static void keyboard_callback(registers_t regs) {
    uint8_t b = inb(0x60);
    printk("Keyboard: %x\n", b & 0xFF);
}

void init_keyboard(void) {
    register_interrupt_handler(IRQ1, &keyboard_callback);
    printk("Init keyboard\n");
}
