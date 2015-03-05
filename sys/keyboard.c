#include <sys/isr.h>
#include <sys/keyboard.h>
#include <sys/screen.h>

#define CTRL_DN 0x1D
#define CTRL_UP 0x9D
#define SHFT_DN 0x2A
#define SHFT_UP 0xAA

void printk(const char *format, ...);

int is_shft_dn = 0;
int is_cntrl_dn = 0;

char* map[] = {
    "??", "??","1","2","3","4","5","6","7","8","9","0","-","=","<-","tab","Q","W","E","R","T",
    "Y","U","I","O","P","[","]","entr","l-ctrl","A","S","D","F","G","H","J","K","L",
    ";","'","`","l-Shft","\\","Z","X","C","V","B","N","M",",",".","/","r-Shft","KP*",
    "l-alt","Space","CpsLk","F1","F2","F3","F4","F5","F6","F7","F8","F9","F10","NumLk",
    "Scrl","KP7","KP8","KP9","KP-","KP4","KP5","KP6","KP+","KP1","KP2","KP3","KP0",
    "KP.","F11","F12"
};

static void keyboard_callback(registers_t regs) {
    //printk("Keyboard: %x\n", b & 0xFF);
    uint8_t b = inb(0x60);

    if(b == CTRL_UP) {
        is_cntrl_dn = 0;
        return;
    } else if(b == CTRL_DN) {
        is_cntrl_dn =1;
        return;
    }

    if(b == SHFT_UP) {
        is_shft_dn = 0;
        return;
    } else if(b == SHFT_DN) {
        is_shft_dn =1;
        return;
    }


    setxy(70, 24);
    if(is_shft_dn) {
        printk("S");
    } else {
        printk(" ");
    }

    //setxy(78, 24);
    if(is_cntrl_dn) {
        printk("^");
    } else {
        printk(" ");
    }


    //setxy(50,24);
    if(b > 0 && b < 87 && b != CTRL_DN && b != CTRL_UP && b != SHFT_DN && b != SHFT_UP)printk("%s          ", map[b & 0xFF]);
}

void init_keyboard(void) {
    register_interrupt_handler(IRQ1, &keyboard_callback);
    printk("Init keyboard\n");
}
