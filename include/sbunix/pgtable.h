#ifndef __KERNEL__
#error "Trying to include kernel code in non-kernel code."
#else
    #ifndef _PGTABLE_H
    #define _PGTABLE_H
        #include <sys/defs.h>
        
        #define MAX_PAGES 8192
        #define WORD_SIZE 32
        #define PAGE_SIZE 0x1000

        void init_free_pg_list(void *physfree);
        uint32_t is_pg_free(int page);
        uint32_t set_pg_free(int page, int free);

        void initializePaging(void);
        void loadPageDirectory(uint64_t *address);
        void enablePaging(void);
        void enablepae(void);
        void disablePaging(void);
    #endif
#endif
