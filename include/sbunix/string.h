#ifndef __KERNEL__
#error "Trying to include kernel code in non-kernel code."
#else
    #ifndef _STRING_H
    #define _STRING_H
        #include <sys/defs.h>
        void memset(void *buff, char value, uint64_t size);
        size_t strlen(const char *str);
        bool strcmp(const char *str1, const char *str2);
        void *memcpy(void *dest, const void *src, size_t n);
        bool begwith(const char *str, const char* match);
        size_t strappend(char *s1, char *s2, char *buf, int len);
    #endif
#endif
