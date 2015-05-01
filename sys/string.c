#define __KERNEL__
#include <sbunix/string.h>
#include <sys/screen.h>

void memset(void *buff, char value, uint64_t size) {
    if(buff != NULL) {
        for(int i = 0; i < size; i++) {
            *((char*)(buff) + i) = value;
        }
    }
}

void *memcpy(void *dest, const void *src, size_t n) {
    if(dest != NULL && src != NULL) {
        for(int i = 0; i < n; i++) {
        panic("got here :D>>>>\n");
            ((char*)dest)[i] = ((char*)src)[i];
        }
    }
    return dest;
}

size_t strlen(const char *s) {
    size_t length = 0;
    if(s != NULL) {
        for(; *s++ != '\0'; length++) ;
    }
    return length;
}

bool strcmp(const char *str1, const char *str2) {
    bool equal = false;
    if(str1 != NULL && str2 != NULL) {
        size_t str1_len = strlen(str1);
        size_t str2_len = strlen(str2);
        if(str1_len == str2_len) {
            equal = true;
            while(*str1 != '\0') {
                if(*str1++ != *str2++) {
                    equal = false;
                    break;
                }
            }
        }
    }
    return equal;
}
