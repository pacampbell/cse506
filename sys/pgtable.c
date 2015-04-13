#define __KERNEL__
#include <sbunix/pgtable.h>
#include <sys/sbunix.h>
#include <sbunix/string.h>

extern char kernmem;

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
 * page: the page count not the page address
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

void initializePaging(uint64_t physbase, uint64_t physfree) {

    int page_index = get_free_page();       // Get a free page from the page allocator
    set_pg_free(page_index, 0);             // Mark the page in use
    // Get the page address
    pml4_t *pml4 = (pml4_t*) pg_to_addr(page_index);
    // Zero out the page
    memset(pml4, 0, PAGE_SIZE);
    // Travese multi-level pt structures to get the page table
    get_pt(pml4, (uint64_t)&kernmem);

    /* TODO: Remap the kernel into the page */
    /* TODO: Remap video memory */
    /* TODO: Set CR3 */
}

pt_t* get_pt(pml4_t *pml4, uint64_t virtual_address) {
    uint64_t pml4_index = extract_pml4(virtual_address);
    uint64_t pdpt_index = extract_directory_ptr(virtual_address);
    uint64_t pd_index = extract_directory(virtual_address);
    // Now begin finding the base address for each step of the walk
    // also zero out lower 12 bits for permissions and copy the rest
    uint64_t pdpt_base_addr = pml4->entries[pml4_index] & 0xFFFFFFFFFFFFF000;
    // Check to see if we have empty entry
    if(pdpt_base_addr == 0x0) {
        // TODO: Get and set pdpt
    }
    uint64_t pd_base_addr = ((pdpt_t*)pdpt_base_addr)->entries[pdpt_index] & 0xFFFFFFFFFFFFF000;
    // Check to see if we have empty entry
    if(pd_base_addr == 0x0) {
        // TODO: Get and set pd
    }
    uint64_t pt_base_addr = ((pd_t*)pd_base_addr)->entries[pd_index] & 0xFFFFFFFFFFFFF000;
    if(pt_base_addr == 0x0) {
        // TODO: Get and set pt
    }
    return (pt_t*)pt_base_addr;
}


inline uint64_t extract_bits(uint64_t virtual_address, unsigned short start, unsigned short end) {
    uint64_t mask = (1 << (end + 1 - start)) - 1;
    return (virtual_address >> start) & mask;
}

inline uint64_t extract_pml4(uint64_t virtual_address) {
    return extract_bits(virtual_address, 39, 47);
}

inline uint64_t extract_directory_ptr(uint64_t virtual_address) {
    return extract_bits(virtual_address, 30, 38);
}

inline uint64_t extract_directory(uint64_t virtual_address) {
    return extract_bits(virtual_address, 21, 29);
}

inline uint64_t extract_table(uint64_t virtual_address) {
    return extract_bits(virtual_address, 12, 20);
}

inline uint64_t extract_offset(uint64_t virtual_address) {
    return extract_bits(virtual_address, 0, 11);
}
