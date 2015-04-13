#define __KERNEL__
#include <sbunix/pgtable.h>
#include <sys/sbunix.h>

uint64_t page_dir_ptr_tab[4] __attribute__((aligned(0x20)));
// 512 entries
uint64_t page_dir[512] __attribute__((aligned(0x1000)));  // must be aligned to page boundary
uint64_t page_tab[512] __attribute__((aligned(0x1000)));

uint32_t *free_pg_list;
void* free_pg_list_end;

void* pg_to_addr(int pg) {
    return (pg * PAGE_SIZE) + ((char*)free_pg_list_end);
}

int addr_to_pg(void* addr) {
    return ((char*)addr - (char*)free_pg_list_end) / PAGE_SIZE;
}

int get_free_page() {
    int pg = 0;

    for(; pg < MAX_PAGES && !is_pg_free(pg); pg++);

    if(pg >= MAX_PAGES)
        return -1;

    return pg;
}

/**
 * physfree: starts the free list at this location
 */
void init_free_pg_list(void *physfree) {
    free_pg_list = (uint32_t *) physfree;

    //get how many words needed for bitmap
    int size = (MAX_PAGES / WORD_SIZE);
    uint32_t all_on = 0xFFFFFFFF;

    for(int i = 0; i < size; i++) {
        *(free_pg_list + i) |= all_on;
    }

    free_pg_list_end = free_pg_list + size;
}

/**
 * page: the page cunt not the page address
 */
uint32_t is_pg_free(int page) {
    int word_num = (page / WORD_SIZE);
    return free_pg_list[word_num] & ~(1 << (page / WORD_SIZE));
}

/**
 * page: count of the page to set
 * free: 0 will mark it not free, anything else sets it free
 */
uint32_t set_pg_free(int page, int free) {
    int word_num = (page / WORD_SIZE);
    if(free) {
        free_pg_list[word_num] |= (1 << (page / WORD_SIZE));
    } else {
        free_pg_list[word_num] &= ~(1 << (page / WORD_SIZE));
    }

    return free_pg_list[word_num];
}

void initializePaging(void) {
    /**
      +-----------------+-------+------+---+---+------+-----+-----+---+
      | Frame Address   | AVAIL | RSVD | D | A | RSVD | U/S | R/W | P |
      +-----------------+-------+------+---+---+------+-----+-----+---+
      */

    // set the page directory into the PDPT and mark it present
    page_dir_ptr_tab[0] = (uint64_t)&page_dir | 1;
    //set the page table into the PD and mark it present/writable
    page_dir[0] = (uint64_t)&page_tab | 3;
    unsigned int i, address = 0;
    for(i = 0; i < 512; i++) {
        page_tab[i] = address | 3; // map address and mark it present/writable
        address = address + 0x1000;
    }

    // Call functions to set up
    disablePaging();
    // enablepae();
    // loadPageDirectory(page_dir_ptr_tab);
    // enablePaging();
}

void disablePaging(void) {
    uint64_t rv = 0;
    __asm__ __volatile__ (
            // "movq $0x1, %%rax;"
            // "cpuid;"
            // "movq %%rax, %0;"
            "movq $0xC0000080, %%rcx;"
            "rdmsr;"
            "movq %%rax, %0;"
            : "=r"(rv)
            :
            : "%rax", "%rcx"
            );

    printk("msr: 0x%x\n", rv);
}

void loadPageDirectory(uint64_t *address) {
    __asm__ __volatile__ (
            "movq %0, %%cr3"
            :
            : "r" (&page_dir_ptr_tab)
            );
}

void enablepae(void) {
    __asm__ __volatile__(
            "movq %%cr4, %%rax;"
            "bts $5, %%rax;"
            "movq %%rax, %%cr4;"
            :
            :
            : "%rax"
            );
}

void enablePaging(void) {
    __asm__ __volatile__(
            "movq %%cr0, %%rax;"
            "movq $0x80000000, %%rdx;"
            "orq %%rdx, %%rax;"
            "movq %%rax, %%cr0;"
            :
            :
            : "%rax", "%rdx"
            );
}
