#define __KERNEL__
#include <sbunix/pgtable.h>
#include <sys/sbunix.h>

uint64_t page_dir_ptr_tab[4] __attribute__((aligned(0x20)));
// 512 entries
uint64_t page_dir[512] __attribute__((aligned(0x1000)));  // must be aligned to page boundary
uint64_t page_tab[512] __attribute__((aligned(0x1000)));

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
