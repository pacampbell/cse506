#define __KERNEL__
#include <sys/sbunix.h>
#include <sbunix/string.h>
#include <stdarg.h>

static char char_table[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B',
    'C', 'D', 'E', 'F'};

#define BASE_10 10
#define BASE_16 16

static void print_base(size_t v, int base, int *counter, int steps) {
    if(v == 0 && steps == 0) {
        putk(char_table[v % base]);
        *counter += 1;
    } else if(v == 0) {
        return;
    } else {
        steps++;
        print_base(v / base, base, counter, steps);
        putk(char_table[v % base]);
        *counter += 1;
    }
}

void printk(const char *format, ...) {
    va_list val;
    int printed = 0;
    // Temps used for printing
    int tmp_int = 0;
    void *ptr = NULL;
    unsigned int tmp_hex = 0;
    char *tmp_cp;
    volatile char *video_memory = (volatile char*)0xb8000;
    // Start getting values for varargs
    va_start(val, format);
    // Loop through the format string
    while(*format) {
        // Look for % sign
        if(*format == '%') {
            switch(*(format + 1)) {
                case '%':
                    *video_memory = '%';
                    video_memory += 2;
                    printed += 1;
                    format++;
                    break;
                case 's':
                    tmp_cp = va_arg(val, char*);
                    for(int i = 0; i < strlen(tmp_cp); ++i) {
                        putk(*(tmp_cp + i));
                    }
                    format++;
                    break;
                case 'd':
                    tmp_int = va_arg(val, int);
                    if(tmp_int < 0) {
                        // Write out the negative sign
                        putk('-');
                        // Get the absolute value of d; invert bits and add 1
                        tmp_int = ~tmp_int + 1;
                    }
                    // Convert the number to decimal
                    print_base(tmp_int, BASE_10, &printed, 0);
                    format++;
                    break;
                case 'p':
                    ptr = va_arg(val, void*);
                    // Convert the number to hex
                    putk('0');
                    putk('x');
                    print_base((size_t)ptr, BASE_16, &printed, 0);
                    format++;
                    break;
                case 'x':
                    tmp_hex = va_arg(val, unsigned int);
                    // Convert the number to hex
                    print_base(tmp_hex, BASE_16, &printed, 0);
                    format++;
                    break;
                case 'c':
                    tmp_int = va_arg(val, int);
                    //write(STDOUT_FILENO, &tmp_int, sizeof(char));
                    putk(tmp_int);
                    format++;
                    break;
                default:
                    printk("\n-----\nyou broke printk\n-----\n");
                    format++;
                    break;
            }
            // Increment onto the format character
        } else {
            putk(*format);
        }
        ++format;
    }
}
