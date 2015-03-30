#define __KERNEL__
#include <sbunix/pgtable.h>
#include <sys/sbunix.h>

// Create the page table directory
uint32_t page_directory[PAGE_DIRECTORY_SIZE] __attribute__((aligned(PAGE_TABLE_ALIGNMENT_BOUNDRY)));
// Create the first page table entry
uint32_t first_page_table[PAGE_TABLE_SIZE] __attribute__((aligned(PAGE_TABLE_ALIGNMENT_BOUNDRY)));

void initializePaging(void) {
    // Initialize page directory
	for(int i = 0; i < PAGE_DIRECTORY_SIZE; i++) {
		/**
		+-----------------+-------+------+---+---+------+-----+-----+---+
		| Frame Address   | AVAIL | RSVD | D | A | RSVD | U/S | R/W | P |
		+-----------------+-------+------+---+---+------+-----+-----+---+
		R/W, Not Present, and Kernel Access only
		*/
		page_directory[i] = 0x00000002;
	}
	// Initialize first level
	for(int i = 0; i < PAGE_TABLE_SIZE; i++) {
		first_page_table[i] = (i * 0x1000) | 3;
	}
    // Call functions to set up
    printk("Page Directory Address: %p\n", page_directory);
    disablePaging();
	// loadPageDirectory(page_directory);
	// enablePaging();
}

void disablePaging(void) {
    __asm__ __volatile__(
        "movq %%cr0, %%rax;"
         "andq $0x7FFFFFFF, %%rax;"
         "movq %%rax, %%cr0;"
         :
         :
         : "%rax"
    );
}
