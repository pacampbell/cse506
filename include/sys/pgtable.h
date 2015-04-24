#ifndef __KERNEL__
#error "Trying to include kernel code in non-kernel code."
#else
    #ifndef _PGTABLE_H
    #define _PGTABLE_H
        #include <sys/sbunix.h>
        #include <sbunix/string.h>
        #include <sys/screen.h>
        #include <sys/defs.h>

        #define MAX_PAGES 8192
        #define WORD_SIZE_BITS 32
        #define PAGE_SIZE 0x1000

        /* Permission bits - Chapter 4-18 Vol 3A */
        #define P 0x1           // Present
        #define RW 0x2          // Read/Write
        #define US 0x4          // User/Supervisor
        #define PWT 0x8         // Page-level write-through
        #define PCD 0x10        // Page-level cache disable
        #define A 0x20          // Accessed



        /* Multi level page table directories */
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
        typedef struct pd_t pd_t;
        typedef struct pt_t pt_t;

        void init_free_pg_list(void *physfree);
        uint32_t is_pg_free(int page);
        uint32_t set_pg_free(int page, int free);
        int get_free_page();
        void* pg_to_addr(uint64_t pg);
        int addr_to_pg(void* addr);
        void set_kern_pg_used(uint64_t beg, uint64_t end);

        void initializePaging(uint64_t physbase, uint64_t physfree);
        void loadPageDirectory(uint64_t *address);
        void* kmalloc_pg(void);
        void kfree_pg(void *address);

        /* Page table helper methods */
        uint64_t extract_pml4(uint64_t virtual_address);
        uint64_t extract_directory_ptr(uint64_t virtual_address);
        uint64_t extract_directory(uint64_t virtual_address);
        uint64_t extract_table(uint64_t virtual_address);
        uint64_t extract_offset(uint64_t virtual_address);
        uint64_t extract_bits(uint64_t virtual_address, unsigned short start, unsigned short end);

        void set_cr3(pml4_t *pml4);
        pml4_t* get_cr3(void);

        /* Macros for translating addresses; similar to linux kernel */
        #define PG_ALIGN 0xFFFFFFFFFFFFF000
        #define VIRTUAL_BASE 0xFFFFFFFFF0000000
        #define VIRTUAL_OFFSET 0xFFFFFFFF80000000               // kernmem in linker script
        #define PHYS_TO_VIRT(physical) (((uint64_t) physical + VIRTUAL_OFFSET))
        #define VIRT_TO_PHYS(virtual) (((uint64_t) virtual - VIRTUAL_OFFSET))

        /**
         * Gets the page table using the virtual address
         * @param pml4 The top level paging structure in IA-32e and IA-64.
         * @param virtual_address The virtual address to translate.
         * @param Returns the address of the page table from the translation.
         */
        pt_t* get_pt(pml4_t *pml4, uint64_t virtual_address);
    #endif
#endif
