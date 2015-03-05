#ifndef __KERNEL__
#error "Trying to include kernel code in non-kernel code."
#else
    #ifndef _STRING_H
    #define _STRING_H
        #include <sys/defs.h>
        void memset(void *buff, char value, uint64_t size);
        size_t strlen(const char *str);
    #endif
#endif
