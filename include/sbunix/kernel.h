#ifndef __KERNEL__
#error "Trying to include kernel code in non-kernel code."
#else
    #ifndef _KERNEL_H
    #define _KERNEL_H
        #include <sys/defs.h>
        // long int strtol(const char *nptr, char **endptr, int base);
        char *itoa(int value, char *str);
    #endif
#endif
