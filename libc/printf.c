#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

static char char_table[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B',
    'C', 'D', 'E', 'F'};

#define BASE_10 10
#define BASE_16 16

static void print_base(int v, int base, int *counter, int steps) {
    if(v == 0 && steps == 0) {
        write(STDOUT_FILENO, char_table + (v % base), sizeof(char));
        *counter += 1;
    } else if(v == 0) {
        return;
    } else {
        steps++;
        print_base(v / base, base, counter, steps);
        write(STDOUT_FILENO, char_table + (v % base), sizeof(char));
        *counter += 1;
    }
}

int printf(const char *format, ...) {
    va_list val;
    int printed = 0;
    // Temps used for printing
    int tmp_int = 0;
    unsigned int tmp_hex = 0;
    char *tmp_cp;
    // Start getting values for varargs
    va_start(val, format);
    // Loop through the format string
    while(*format) {
        // Look for % sign
        if(*format == '%') {
            switch(*(format + 1)) {
                case '%':
                    write(STDOUT_FILENO, "%", sizeof(char));
                    printed += 1;
                    break;
                case 's':
                    tmp_cp = va_arg(val, char*);
                    write(STDOUT_FILENO, tmp_cp, strlen(tmp_cp));
                    printed += strlen(tmp_cp);
                    break;
                case 'd':
                    tmp_int = va_arg(val, int);
                    if(tmp_int < 0) {
                        // Write out the negative sign
                        write(STDOUT_FILENO, "-", sizeof(char));
                        // Get the absolute value of d; invert bits and add 1
                        tmp_int = ~tmp_int + 1;
                    }
                    // Convert the number to decimal
                    print_base(tmp_int, BASE_10, &printed, 0);
                    break;
                case 'x':
                    tmp_hex = va_arg(val, unsigned int);
                    // Convert the number to hex
                    print_base(tmp_hex, BASE_16, &printed, 0);
                    break;
                case 'c':
                    tmp_int = va_arg(val, int);
                    write(STDOUT_FILENO, &tmp_int, sizeof(char));
                    break;
                default:
                    printf("\n-----\nyou broke printf\n-----\n");
                    return -1;
                    break;
            }
            // Increment onto the format character
            ++format;
        } else {
            write(STDOUT_FILENO, format, sizeof(char));
            printed++;
        }
        ++format;
    }
    return printed;
}
