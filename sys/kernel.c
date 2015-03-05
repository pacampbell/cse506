#define __KERNEL__
#include <sbunix/kernel.h>
#include <sbunix/string.h>

char *itoa(int value, char *str) {
    if(str != NULL) {
        int is_neg = 0;
        if (value < 0) {
            is_neg = 1;
            value = ~value + 1;
        }
        int i = 0;
        do {
            str[i++] = (value % 10) + '0';
            value = value / 10;
        } while(value);
        if(is_neg) str[i++] = '-';
        // NULL terminate the string
        str[i] = '\0';
        // Reverse the string
        for(int start = 0, end = strlen(str) - 1; start < end; start++, end--) {
            str[start] ^= str[end];
            str[end] ^= str[start];
            str[start] ^= str[end];
        }
    }
    return str;
}
