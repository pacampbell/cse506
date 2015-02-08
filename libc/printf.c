#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

int printf(const char *format, ...) {
	va_list val;
	int printed = 0;
        char *tmp_cp;
        int tmp_i;

	va_start(val, format);

        while(*format) {
            if(*format == '%') {
                format++;

                switch(*format) {
                    case 's':
                        tmp_cp = va_arg(val, char*);
                        while(*tmp_cp) {
                            write(1, tmp_cp, 1);
                            tmp_cp++;
                        }
                        format++;
                        break;

                    case 'd':
                        tmp_i = va_arg(val, int);
                        putint(tmp_i);
                        format++;
                        break;

                    default:
                        printf("\n-----\nyou broke printf\n-----\n");
                        return -1;
                        break;
                }
            }
            write(1, format, 1);
            ++printed;
            ++format;
        }

        return printed;
}
