#ifndef __KERNEL__
#error "Trying to include kernel code in non-kernel code."
#else
    #ifndef _PGTABLE_H
    #define _PGTABLE_H
        #include <sys/defs.h>

        #define PAGE_DIRECTORY_SIZE 1024
        #define PAGE_TABLE_SIZE 1024
        #define PAGE_TABLE_ALIGNMENT_BOUNDRY 4096

        void initializePaging(void);
        void loadPageDirectory(unsigned int *address);
        void enablePaging(void);
        void disablePaging(void);
    #endif
#endif
