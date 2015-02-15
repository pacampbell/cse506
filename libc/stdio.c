#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#define BASE_10 10
#define BASE_16 16

static int convert(const char *str, int base) {
// TODO: Check negative sign
    int val = 0;
    if(str != NULL && base > 1) {
        for(int i = 0; str[i] != '\0'; i++) {
            if(str[i] >= '0' && str[i] <= '9') {
                val = base * val + (str[i] - '0');
            } else if(base > BASE_10 && str[i] >= 'A' && str[i] <= 'F') {
                val = base * val + (str[i] - 'A' + 10);
            } else if(base > BASE_10 && str[i] >= 'a' && str[i] <= 'f') {
                val = base * val + (str[i] - 'a' + 10);
            } else {
                // bad base; stop parsing
                break;
            }
        }
    }
    return val;
}

char *fgets(char *s, int size, int fd) {
    char c;
    int i;

    for(i = 0; i < size; i++) {
        if(read(fd, &c, 1) == 1) {
            switch(c) {
                case '\n':
                case '\0':
                    *(s + i) = '\0';
                    goto done;
                default:
                    *(s + i) = c;
                    break;
            }

        } else {
            return NULL;
        }

    }

done:
    if(i == 0) return NULL;
    return s;

}

#define BUFFER_SIZE 1024

int scanf(const char *format, ...) {
    va_list val;
    int got = 0;
    char *tmp_cp;
    int *in_c;
    // Store user input
    char buffer[BUFFER_SIZE];
    char token[BUFFER_SIZE];
    int in = 0;
    int i = 0, j = 0;
    char c;

    va_start(val, format);

    in = read(STDIN_FILENO, &c, sizeof(char));
    while(in > 0 && c != '\0' && c != ' ' && c != '\n') {
        buffer[i++] = c;
        in = read(STDIN_FILENO, &c, sizeof(char));
    }
    // Null terminate the string
    buffer[i] = '\0';
    j = 0;
    while(*format != '\0' && *format != '\n') {
        if(*format == '%') {
            i = 0;
            // Read the next token
            while(buffer[j] != ' ' && buffer[j] != '\0') {
                token[i++] = buffer[j++];
            }
            // Null terminate the token
            token[i] = '\0';
            switch(*(format + 1)) {
                case 's':
                    tmp_cp = va_arg(val, char*);
                    strcpy(tmp_cp, token);
                    break;
                case 'd':
                    in_c = va_arg(val, int*);
                    *in_c = convert(token, BASE_10);
                    break;
                case 'x':
                    in_c = va_arg(val, int*);
                    *in_c = convert(token, BASE_16);
                    break;
                case 'c':
                    break;
                default:
                    /* Bad shouldnt get here */
                    break;
            }
            // Increment to the specifier
            format++;
        }
        format++;
    }
    return got;
}
