#define __KERNEL__
#include <sys/isr.h>
#include <sys/keyboard.h>
#include <sys/screen.h>

#define CTRL_DN 0x1D
#define CTRL_UP 0x9D
#define SHFT_DN 0x2A
#define SHFT_UP 0xAA
#define ENTER   0x9C
#define BSPACE  0xE

#define READ_SIZE 50

#define KEY_MASK 0xff
#define STDIN_BUFFER_SIZE 2048

void printk(const char *format, ...);

static bool is_shft_dn = false;
static bool is_cntrl_dn = false;
static volatile bool pressed_enter = false;
static volatile bool is_reading = false;
volatile char* curs;
volatile char* at;
//static char buf[READ_SIZE];


/* Character table mappings for the scan codes */
char* map[] = {
    "??", "??","1","2","3","4","5","6","7","8","9","0","-","=","","\t","q","w","e","r","t",
    "y","u","i","o","p","[","]","\n","l-ctrl","a","s","d","f","g","h","j","k","l",
    ";","'","`","l-Shft","\\","z","x","c","v","b","n","m",",",".","/","r-Shft","KP*",
    "l-alt"," ","CpsLk","F1","F2","F3","F4","F5","F6","F7","F8","F9","F10","NumLk",
    "Scrl","KP7","KP8","KP9","KP-","KP4","KP5","KP6","KP+","KP1","KP2","KP3","KP0",
    "KP.","F11","F12"
};

char* s_map[] = {
    "??", "??","!","@","#","$","%","^","&","*","(",")","_","+","","\t","Q","W","E","R","T",
    "Y","U","I","O","P","{","}","\n","l-ctrl","A","S","D","F","G","H","J","K","L",
    ":","\"","~","l-Shft","|","Z","X","C","V","B","N","M","<",">","?","r-Shft","KP*",
    "l-alt"," ","CpsLk","F1","F2","F3","F4","F5","F6","F7","F8","F9","F10","NumLk",
    "Scrl","KP7","KP8","KP9","KP-","KP4","KP5","KP6","KP+","KP1","KP2","KP3","KP0",
    "KP.","F11","F12"
};

// static char stdin_buffer[STDIN_BUFFER_SIZE];
// static uint64_t buffer_index = 0;
// static char *last_read_ptr = stdin_buffer;
// static char *last_insert_ptr = stdin_buffer;

// static ssize_t fill_buffer(char *str) {
//     ssize_t filled = 0;
//     if(str != NULL) {
//         while(*str != '\0') {
//             //stdin_buffer[buffer_index];
//             str++;
//         }
//     }
//     return filled;
// }

int gets(uint64_t addr, size_t len) {
    if (0 && len > READ_SIZE) {
        panic("gets len too big\n");
        return -1;
    }
    int count = 0;
    curs = get_cursor();
    pressed_enter = false;
    is_reading = true;
    __asm__ __volatile__("sti;");
    
    while (pressed_enter == false) {
        __asm__ __volatile__("hlt;");
    }
    is_reading = false;

    while ( *curs != '\n' && *curs != '\0' && count < (len - 1 ) ) {
        *((volatile char*)(addr++)) = *curs;
        count++;
        curs++;
        curs++;
    }
    *((volatile char*)(addr)) = '\0';

    return count;
}

void keyboard_callback(registers_t regs) {
    uint8_t b = inb(0x60);
    //printk("key: %p\n", b&KEY_MASK);
    switch(b) {
        case CTRL_UP:
            is_cntrl_dn = false;
            break;
        case CTRL_DN:
            is_cntrl_dn = true;
            break;
        case SHFT_UP:
            is_shft_dn = false;
            break;
        case SHFT_DN:
            is_shft_dn = true;
            break;
        default:
            if((b & KEY_MASK) < 87 && (b & KEY_MASK) >= 0)  {
                if((b & KEY_MASK) == BSPACE && is_reading && at > curs) {
                    printk("%s", "\b \b");
                } else {
                    if(is_shft_dn) {
                        printk("%s", s_map[b & 0xFF]);
                    } else {
                        printk("%s", map[b & 0xFF]);
                    }
                }
            }
            break;
    }

    at = get_cursor();
    if (b == ENTER) pressed_enter = true;
}

void init_keyboard(void) {
    register_interrupt_handler(IRQ1, &keyboard_callback);
}
