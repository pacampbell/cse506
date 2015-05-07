#define __KERNEL__
#include <sys/isr.h>
#include <sys/keyboard.h>
#include <sys/screen.h>

#define CTRL_DN 0x1D
#define CTRL_UP 0x9D
#define SHFT_DN 0x2A
#define SHFT_UP 0xAA
#define ENTER   0x9C

void printk(const char *format, ...);

int is_shft_dn = 0;
int is_cntrl_dn = 0;
static volatile bool pressed_enter= false;

char* map[] = {
    "??", "??","1","2","3","4","5","6","7","8","9","0","-","=","<-","tab","Q","W","E","R","T",
    "Y","U","I","O","P","[","]","entr","l-ctrl","A","S","D","F","G","H","J","K","L",
    ";","'","`","l-Shft","\\","Z","X","C","V","B","N","M",",",".","/","r-Shft","KP*",
    "l-alt","Space","CpsLk","F1","F2","F3","F4","F5","F6","F7","F8","F9","F10","NumLk",
    "Scrl","KP7","KP8","KP9","KP-","KP4","KP5","KP6","KP+","KP1","KP2","KP3","KP0",
    "KP.","F11","F12"
};

int gets(uint64_t addr, size_t len) {
    __asm__ __volatile__("sti;");
    int count = 0;
    volatile char* curs = get_cursor();
    pressed_enter = 0;
    
    while (pressed_enter == false) {
        printk("loop: %d\n", count++);
        __asm__ __volatile__("hlt;");
    }

    while ( *curs != '\n' && *curs != '\0' && count < (len - 1 ) ) {
        *((volatile char*)(addr++)) = *curs;
        count++;
    }

    addr = '\0';

    return count;
}

static void keyboard_callback(registers_t regs) {
    uint8_t b = inb(0x60);
    //printk("Keyboard: %x\n", b & 0xFF);
    
    if  ((b & 0xFF) == ENTER) {
        pressed_enter = true;
    }

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
    //printk("Init keyboard\n");
}
