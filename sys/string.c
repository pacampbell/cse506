#define __KERNEL__
#include <sbunix/string.h>

void memset(void *buff, char value, uint64_t size) {
    if(buff != NULL) {
        for(int i = 0; i < size; i++) {
            *((char*)(buff) + i) = value;
        }
    }
}

size_t strlen(const char *s) {
    size_t length = 0;
    if(s != NULL) {
        for(; *s++ != '\0'; length++) ;
    }
    return length;
}
