#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

static char char_table[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B',
    'C', 'D', 'E', 'F'};

#define BASE_10 10
#define BASE_16 16

static void print_base(int fd, int v, int base, int *counter, int steps) {
    if(v == 0 && steps == 0) {
        write(fd, char_table + (v % base), sizeof(char));
        *counter += 1;
    } else if(v == 0) {
        return;
    } else {
        steps++;
        print_base(fd, v / base, base, counter, steps);
        write(fd, char_table + (v % base), sizeof(char));
        *counter += 1;
    }
}

int fprintf(int fd, const char *format, ...) {
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
                    write(fd, "%", sizeof(char));
                    printed += 1;
                    break;
                case 's':
                    tmp_cp = va_arg(val, char*);
                    write(fd, tmp_cp, strlen(tmp_cp));
                    printed += strlen(tmp_cp);
                    break;
                case 'd':
                    tmp_int = va_arg(val, int);
                    if(tmp_int < 0) {
                        // Write out the negative sign
                        write(fd, "-", sizeof(char));
                        // Get the absolute value of d; invert bits and add 1
                        tmp_int = ~tmp_int + 1;
                    }
                    // Convert the number to decimal
                    print_base(fd, tmp_int, BASE_10, &printed, 0);
                    break;
                case 'x':
                    tmp_hex = va_arg(val, unsigned int);
                    // Convert the number to hex
                    print_base(fd, tmp_hex, BASE_16, &printed, 0);
                    break;
                case 'c':
                    tmp_int = va_arg(val, int);
                    write(fd, &tmp_int, sizeof(char));
                    break;
                default:
                    printf("\n-----\nyou broke printf\n-----\n");
                    return -1;
                    break;
            }
            // Increment onto the format character
            ++format;
        } else {
            write(fd, format, sizeof(char));
            printed++;
        }
        ++format;
    }
    return printed;
}

int printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    return fprintf(STDOUT_FILENO, format, args);
}

