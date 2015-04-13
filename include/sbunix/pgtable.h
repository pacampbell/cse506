#ifndef __KERNEL__
#error "Trying to include kernel code in non-kernel code."
#else
    #ifndef _PGTABLE_H
    #define _PGTABLE_H
        #include <sys/defs.h>

        #define MAX_PAGES 8192
        #define WORD_SIZE 32
        #define PAGE_SIZE 0x1000

        struct pml4_t {
            uint64_t entries[512];
        };

        struct pdpt_t {
            uint64_t entries[512];
        };

        struct pd_t {
            uint64_t entries[512];
        };

        struct pt_t {
            uint64_t entries[512];
        };

        typedef struct pml4_t pml4_t;
        typedef struct pdpt_t pdpt_t;
        typedef struct pde_t pde_t;
        typedef struct pt_t pt_t;

        void init_free_pg_list(void *physfree);
        uint32_t is_pg_free(int page);
        uint32_t set_pg_free(int page, int free);
        int get_free_page();
        void* pg_to_addr(int pg);
        int addr_to_pg(void* addr);

        void initializePaging(uint64_t physbase, uint64_t physfree);
        void loadPageDirectory(uint64_t *address);

        /* Page table helper methods */
        uint64_t extract_pml4(uint64_t virtual_address);
        uint64_t extract_directory_ptr(uint64_t virtual_address);
        uint64_t extract_directory(uint64_t virtual_address);
        uint64_t extract_table(uint64_t virtual_address);
        uint64_t extract_offset(uint64_t virtual_address);
        uint64_t extract_bits(uint64_t virtual_address, unsigned short start, unsigned short end);

        /**
         * Gets the page table using the virtual address
         * @param pml4 The top level paging structure in IA-32e and IA-64.
         * @param virtual_address The virtual address to translate.
         * @param Returns the address of the page table from the translation.
         */
        uint64_t get_pt(uint64_t pml4, unit64_t virtual_address);
    #endif
#endif
