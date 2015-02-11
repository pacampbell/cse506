#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

void putint(int i) {
    char buffer[256];
    char nl = '\n';
    int count = 0;
    if(i != 0) {
        // Get ascii digits in reverse
        while(i > 0) {
            buffer[count++] = (i % 10) + '0';
            i /= 10;
        }
        count--;
        // Print out the ascii digits 1 at a time
        while(count >= 0) {
            write(STDOUT_FILENO, &buffer[count--], 1);
        }
    } else {
        char z = '0';
        write(STDOUT_FILENO, &z, 1);
    }
    write(STDOUT_FILENO, &nl, 1);
}

int scanf(const char *format, ...) {
	va_list val;
	int got = 0;

        char *tmp_cp;
        char in_c;

	va_start(val, format);

        while(*format) {
            if(*format == '%') {
                format++;

                switch(*format) {
                    case 's':
                        tmp_cp = va_arg(val, char*);

                        do {
                            read(STDIN_FILENO, &in_c, 1);
                            *tmp_cp = in_c;
                            tmp_cp++;
                        } while(in_c != '\0' && in_c != '\n');

                        if(in_c == '\n') {
                            *tmp_cp = '\0';
                        }

                        format++;
                        break;
                /*
                    case 'd':
                        format++;
                        break;
                */

                    default:
                        printf("\n-----\nyou broke scanf in switch\n-----\n");
                        return -1;
                        break;
                }
            } else {
                printf("\n-----\nyou broke scanf in else\n-----\n");
            }

            ++got;
        }

        return got;
}
