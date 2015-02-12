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

char *strtok(char *str, const char delim) {
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

char *strappend(char *s1, char *s2, char *s3) {
    static char buf[256];
    char *cb = buf;
    char *cp = s1;

    for(cp = s1; *cp != '\0'; cp++, cb++) {
        *cb = *cp;
    }

    for(cp = s2; *cp != '\0'; cp++, cb++) {
        *cb = *cp;
    }

    for(cp = s3; *cp != '\0'; cp++, cb++) {
        *cb = *cp;
    }

    *cb = '\0';

    return buf;
}

char *strcpy(char *dst, const char *src) {
    if(src != NULL && dst != NULL) {
        while(*src != '\0') {
            *dst++ = *src++;
        }
        *dst = '\0';
    }
    return dst;
}

char *strip(char *src) {
    if(src != NULL) {
        // Remove preceding spaces
        while(*src == ' ') {
            src++;
        }
        // Remove traling spaces
        int end = strlen(src) - 1;
        while(src[end] == ' ' || src[end] == '\n') {
            end--;
        }
        // Add a null terminator to the new end
        src[end + 1] = '\0';
    }
    return src;
}

static int count_tokens(const char *cmd, char token) {
    int tokens = 0;
    if(cmd != NULL) {
        while(*cmd != '\0') {
            if(*cmd++ == token) tokens++;
        }
        // if theres n vertices there is n + 1 nodes
        tokens++;
    }
    return tokens;
}

char **split(const char *str, char delim) {
    char **tokens = NULL;
    if(str != NULL) {
        // Count tokens
        int count = count_tokens(str, delim);
        // Allocate space
        int str_len = strlen(str);
        char *cpy = malloc(str_len + 1);
        // Copy the string
        strcpy(cpy, str);
        // Allocate space for all tokens
        tokens = malloc(sizeof(char*) * (count + 1));
        // Begin splitting the string
        char **tkn_ptr = tokens;
        *tkn_ptr++ = cpy;   // Set the first token
        while(*cpy != '\0') {
            if(*cpy == delim) {
                *cpy++ = '\0';
                *tkn_ptr++ = cpy;
            }
            cpy++;
        }
        // Set the last element as NULL so we can find the bottom
        *tkn_ptr = NULL;
    }
    return tokens;
}
