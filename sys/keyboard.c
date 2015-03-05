#include <sys/isr.h>
#include <sys/keyboard.h>

void printk(const char *format, ...);

char* map[] = {
    "??", "??","1","2","3","4","5","6","7","8","9","0","-","=","<-","tab","Q","W","E","R","T",
    "Y","U","I","O","P","[","]","entr","l-ctrl","A","S","D","F","G","H","J","K","L",
    ";","'","`","l-Shft","\\","Z","X","C","V","B","N","M",",",".","/","r-Shft","KP*",
    "l-alt","Space","CpsLk","F1","F2","F3","F4","F5","F6","F7","F8","F9","F10","NumLk",
    "Scrl","KP7","KP8","KP9","KP-","KP4","KP5","KP6","KP+","KP1","KP2","KP3","KP0",
    "KP.","F11","F12"
};

static void keyboard_callback(registers_t regs) {
    uint8_t b = inb(0x60);
    //printk("Keyboard: %x\n", b & 0xFF);
    if(b > 0 && b < 87)printk("got: %s\n", map[b & 0xFF]);
}

void init_keyboard(void) {
    register_interrupt_handler(IRQ1, &keyboard_callback);
    printk("Init keyboard\n");
}
