#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

int scanf(const char *format, ...) {
	va_list val;
	int got = 0;

        char *tmp_cp;
        char in_c;

	va_start(val, format);

        while(*format) {
            if(*format == '%') {
                switch(*(format + 1)) {
                    case '%':

                        break;
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
                    case 'd':
                        format++;
                        break;
                    case 'x':
                        break;
                    case 'c':
                        break;

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
