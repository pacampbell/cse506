#ifndef __KERNEL__
#error "Trying to include kernel code in non-kernel code."
#else
    #ifndef _KMAIN_H
    #define _KMAIN_H
        void kmain(void);
    #endif
#endif
