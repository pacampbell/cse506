#ifndef __KERNEL__
#error "Trying to include kernel code in non-kernel code."
#else
    #ifndef _PGTABLE_H
    #define _PGTABLE_H
        #include <sys/defs.h>

        void initializePaging(void);
        void loadPageDirectory(uint64_t *address);
        void enablePaging(void);
        void enablepae(void);
        void disablePaging(void);
    #endif
#endif
