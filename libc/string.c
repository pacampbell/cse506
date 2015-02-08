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

void *memcpy(void *dest, const void *src, size_t n) {
    if(dest != NULL && src != NULL) {
        for(int i = 0; i < n; i++) {
            ((char*)dest)[i] = ((char*)src)[i];
        }
    }
    return dest;
}

//string begins with
int strbegwith(char *str1, char *str2) {
    char *c1, *c2;
    for(c1 = str1, c2 = str2; *c1 != 0 && *c2 != 0; c1++, c2++) {
        if(*c1 != *c2) {
            return 0;
        }
    }
    return 1;
}

char *strtok(char *str, char delim) {
    static char *ptr = NULL;
    static char *head = NULL;
    if(str != NULL) {
        head = str;
        ptr = str;
    } else {
        head = ptr;
    }

    if(head != NULL) {
        while(*ptr != delim && *ptr != '\0') {
            ptr++;
        }
        if(*ptr == '\0') {
            ptr = NULL;
        } else {
            *ptr++ = '\0';
        }
    }
    return head;
}
