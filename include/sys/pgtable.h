#ifndef __KERNEL__
#error "Trying to include kernel code in non-kernel code."
#else
    #ifndef _PGTABLE_H
    #define _PGTABLE_H
        #include <sys/sbunix.h>
        #include <sbunix/string.h>
        #include <sys/screen.h>
        #include <sys/defs.h>
        
        #define FREE 1
        #define USED 0

        #define MAX_PAGES 8192
        #define WORD_SIZE_BITS 32
        #define PAGE_SIZE 0x1000
        #define MAX_TABLE_ENTRIES 512

        /* Copy on write - Use an unused bit to represent COW */
        #define COW 0x1000000000000000
        
        /* Permission bits - Chapter 4-18 Vol 3A */
        #define P 0x1           // Present
        #define RW 0x2          // Read/Write
        #define US 0x4          // User/Supervisor
        #define PWT 0x8         // Page-level write-through
        #define PCD 0x10        // Page-level cache disable
        #define A 0x20          // Accessed

        /* Helper constants for paging */
        #define USER_SETTINGS (P | RW | US)
        #define KERN_SETTINGS (P | RW) 

        #define RECURSIVE_SLOT (510L)

        #define L4_SHIFT (39)
        #define L3_SHIFT (30)
        #define L2_SHIFT (21)
        #define L1_SHIFT (12)

#define UPPER_ADDR(x) ( (uint64_t*) (0xffffL << 48 | (x) ) )

#define PGTBL_ADDR UPPER_ADDR((RECURSIVE_SLOT<<L4_SHIFT))

#define PGDIR_ADDR UPPER_ADDR((RECURSIVE_SLOT<<L4_SHIFT) \
                     |(RECURSIVE_SLOT<<L3_SHIFT))

#define PDPT_ADDR UPPER_ADDR((RECURSIVE_SLOT<<L4_SHIFT) \
                     |(RECURSIVE_SLOT<<L3_SHIFT) \
                     |(RECURSIVE_SLOT<<L2_SHIFT))

#define PML4T_ADDR UPPER_ADDR((RECURSIVE_SLOT<<L4_SHIFT) \
                     |(RECURSIVE_SLOT<<L3_SHIFT) \
                     |(RECURSIVE_SLOT<<L2_SHIFT) \
                     |(RECURSIVE_SLOT<<L1_SHIFT))

       

         
        /* Multi level page table directories */
        struct pml4_t {
            uint64_t entries[MAX_TABLE_ENTRIES];
        };

        struct pdpt_t {
            uint64_t entries[MAX_TABLE_ENTRIES];
        };

        struct pd_t {
            uint64_t entries[MAX_TABLE_ENTRIES];
        };

        struct pt_t {
            uint64_t entries[MAX_TABLE_ENTRIES];
        };

        typedef struct pml4_t pml4_t;
        typedef struct pdpt_t pdpt_t;
        typedef struct pd_t pd_t;
        typedef struct pt_t pt_t;

        void mark_address_range_free(uint64_t start, uint64_t end, int free);
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
        void* kmalloc_vma(pml4_t *cr3, uint64_t virt_base, size_t size, uint64_t permissions);
        void *kmalloc_kern(size_t size);
        bool leaks_pg(uint64_t virt_base, size_t size);
        uint64_t calculate_total_page_by_size(size_t size, uint64_t base_addr);

        /**
         * Makes a copy of a src page table.
         * @param src Page table pml4 to start copying.
         * @return Returns a copy of src.
         */
        pml4_t* copy_page_tables(pml4_t *src);

        /**
         * Inserts a page into the page table referenced by cr3
         * mapped by the virtual address;
         * @param cr3 The physical pml4 address.
         * @param virtual_address Virtual address to map the page to.
         * @param permissions Permissions for the lower 12 bits of pte.
         * @return Returns the virtual address of the newly inserted page.
         */
        uint64_t insert_page(pml4_t *cr3, uint64_t virtual_address, uint64_t permissions);

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
        #define PG_ALIGN (0xFFFFFFFFFFFFF000)
        #define VIRTUAL_BASE (0xFFFFFFFFF0000000)
        #define VIRTUAL_OFFSET (0xFFFFFFFF80000000)               // kernmem in linker script
        #define PHYS_TO_VIRT(physical) (((uint64_t) (physical) + VIRTUAL_OFFSET))
        #define VIRT_TO_PHYS(virtual) (((uint64_t) (virtual) - VIRTUAL_OFFSET))

        #define PG_RND_UP(address) (((uint64_t)(address) + PAGE_SIZE) & PG_ALIGN)
        #define PG_RND_DOWN(address) (((uint64_t)(address) & PG_ALIGN))

        #define FLUSH_TLB(virtual) do {             \
            __asm__ __volatile__(                   \
                "invlpg (%0)"                       \
                :                                   \
                : "b"((void*)(virtual))             \
                : "memory"                          \
            );                                      \
        } while(0)

        /**
         * Walks a page table and prints out all the contents.
         */
        void dump_tables(pml4_t *cr3);
        void check_vma_permissions(pml4_t *cr3, uint64_t address);

        // pdpt_t* get_pml4e(pml4_t *cr3, uint64_t virtual_address);
        // pd_t* get_pdpte(pml4_t *cr3, uint64_t virtual_address);
        // pt_t* get_pde(pml4_t *cr3, uint64_t virtual_address);
        // uint64_t get_pte(pml4_t *cr3, uint64_t virtual_address);

        /* Cleaner insertion functions */
        // pdpt_t* insert_pml4e(pml4_t *cr3, uint64_t virtual_address);
        // pd_t* insert_pdpte(pml4_t *cr3, uint64_t virtual_address);
        // pt_t* insert_pde(pml4_t *cr3, uint64_t virtual_address);
        // uint64_t insert_pte(pml4_t *cr3, uint64_t virtual_address, uint64_t permissions);

        /**
         * Gets the page table using the virtual address
         * @param pml4 The top level paging structure in IA-32e and IA-64.
         * @param virtual_address The virtual address to translate.
         * @param Returns the address of the page table from the translation.
         */
        pt_t* get_pt(pml4_t *pml4, uint64_t virtual_address);
        pt_t* get_pt_virt(pml4_t *pml4, uint64_t virtual_address, uint64_t permissions);

        /* Self referencing */
        uint64_t insert_pml4e(pml4_t *cr3, uint64_t virtual_address, uint64_t frame);
        uint64_t insert_pdpte(pml4_t *cr3, uint64_t virtual_address, uint64_t frame);
        uint64_t insert_pde(pml4_t *cr3, uint64_t virtual_address, uint64_t frame);
        uint64_t insert_pte(pml4_t *cr3, uint64_t virtual_address, uint64_t frame);
        uint64_t get_pml4e(pml4_t *cr3, uint64_t virtual_address);
        uint64_t get_pdpte(pml4_t *cr3, uint64_t virtual_address);
        uint64_t get_pde(pml4_t *cr3, uint64_t virtual_address);
        uint64_t get_pte(pml4_t *cr3, uint64_t virtual_address);
    #endif
#endif
