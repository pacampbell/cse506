#include <string.h>

size_t strlen(const char *s) {
    size_t length = 0;
    if(s != NULL) {
        for(; *s++ != '\0'; length++) ;
    }
    return length;
}

void *memset(void *s, int c, size_t n) {
    if(s != NULL) {
        for(int i = 0; i < n; i++) {
            *(char*)s = c;
        }
    }
    return s;
}