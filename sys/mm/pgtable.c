#define __KERNEL__
#include <sys/pgtable.h>

extern char kernmem;
extern void *kern_free;
extern void *kern_base;

uint32_t *free_pg_list;
void* free_pg_list_end;

void* pg_to_addr(uint64_t pg) {
    //return (void*)((pg * PAGE_SIZE) + ((uint64_t)free_pg_list_end));
    return (void*)((pg * PAGE_SIZE) + ((uint64_t)kern_base));
}

int addr_to_pg(void* addr) {
    //return ((uint64_t)addr - (uint64_t)free_pg_list_end) / PAGE_SIZE;
    return ((uint64_t)addr - (uint64_t)kern_base) / PAGE_SIZE;
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
    int size = (MAX_PAGES / WORD_SIZE_BITS);
    uint32_t all_on = 0xFFFFFFFF;

    for(int i = 0; i < size; i++) {
        *(free_pg_list + i) |= all_on;
    }

    free_pg_list_end = free_pg_list + size;

    kern_free += (uint64_t)(sizeof(char) * MAX_PAGES);
}

/**
 * page: the page count not the page address
 */
uint32_t is_pg_free(int page) {
    int word_num = (page / WORD_SIZE_BITS);
    return free_pg_list[word_num] >> (page % WORD_SIZE_BITS) & 0x1;
}

/**
 * page: count of the page to set
 * free: 0 will mark it not free, anything else sets it free
 */
uint32_t set_pg_free(int page, int free) {
    int word_num = (page / WORD_SIZE_BITS);
    if(free) {
        free_pg_list[word_num] |= (1 << (page % WORD_SIZE_BITS));
    } else {
        free_pg_list[word_num] &= ~(1 << (page % WORD_SIZE_BITS));
    }

    return free_pg_list[word_num];
}

void set_kern_pg_used(uint64_t beg, uint64_t end) {
    int beg_i = beg / PAGE_SIZE;
    int end_i = end / PAGE_SIZE;

    for(; beg_i < end_i; beg_i++) {
        int condition = (uint64_t)kern_base <= (beg_i * PAGE_SIZE) && (beg_i * PAGE_SIZE) <= (uint64_t)kern_free;
        set_pg_free(beg_i, condition);
    }
}

void initializePaging(uint64_t physbase, uint64_t physfree) {
    // Get a new page to setup the pml4
    pml4_t *pml4 = (pml4_t*) kmalloc_pg();
    // Zero out the page
    memset(pml4, 0, PAGE_SIZE);
    // Travese multi-level pt structures to get the page table
    uint64_t kernel_virtual_address = (uint64_t)&kernmem;
    pt_t* page_table = get_pt(pml4, kernel_virtual_address);
    /* Remap the kernel */
    uint64_t kern_physbase = (uint64_t)kern_base;
	uint64_t kern_physfree = (uint64_t)kern_free;
    uint64_t kern_virt_base_addr = kernel_virtual_address & VIRTUAL_BASE;
    // Loop through and set values
    uint64_t kern_vma = 0;
    while(kern_physbase <= kern_physfree) {
        kern_vma = kern_virt_base_addr | kern_physbase;
        page_table->entries[extract_table(kern_vma)] = kern_physbase | KERN_SETTINGS;
        kern_physbase += PAGE_SIZE;
    }
    /* Remap video memory; its only 4000 bytes so fits in 1 page */
    uint64_t video_phybase = VIDEO_MEM_START;
    // uint64_t video_physfree = video_mem_limit;
    uint64_t video_vma = video_phybase | kern_virt_base_addr;
    // Get the page table
    page_table = get_pt(pml4, video_vma);
    // Set video mem page permissions
    page_table->entries[extract_table(video_vma)] = video_phybase | KERN_SETTINGS;
    // Increment the kernel memory by 1 more page
    kern_physbase += PAGE_SIZE;
    /* Set CR3 */
    // pml4->entries[510] = (uint64_t)pml4 | P | RW | US; // self referencing impl...
    // Set CR3
    set_cr3(pml4);
    // tell the print driver to use new address
    map_video_mem(video_vma);
    // reset freelist and videomemory pointers
    free_pg_list = (uint32_t*) PHYS_TO_VIRT(free_pg_list);
    printk("Remapped kernel mem [%p:%p]\n", kernel_virtual_address, kern_vma);
    printk("Remapped video mem [%p:%p]\n", video_vma, video_vma + 4000);
}

pt_t* get_pt_virt(pml4_t *pml4, uint64_t virtual_address, uint64_t permissions) {
    uint64_t pml4_index = extract_pml4(virtual_address);
    uint64_t pdpt_index = extract_directory_ptr(virtual_address);
    uint64_t pd_index = extract_directory(virtual_address);
    // Store the current cr3 and set the new one
    // pml4_t *old_cr3 = get_cr3();
    // halt();
    // set_cr3(pml4);
    // printk("virtual_address: %p\n", virtual_address);
    // Now begin finding the base address for each step of the walk
    // also zero out lower 12 bits for permissions and copy the rest
    // printk("pml4 index: %d\n", pml4_index);
    uint64_t pdpt_base_addr = pml4->entries[pml4_index] & PG_ALIGN;
    // printk("pdpt_base_addr: %p\n", pdpt_base_addr);
    // Check to see if we have empty entry
    if(pdpt_base_addr == 0x0) {
        // Get a new page
        pdpt_t *page = (pdpt_t*) PHYS_TO_VIRT(kmalloc_pg());
        // Move the kernel free pointer by 1 page
        kern_free = kern_free + PAGE_SIZE;
        // Zero out the memory
        memset(page, 0, PAGE_SIZE);
        // Set the page into the current index and set permissions in the lower 12 bits
        pml4->entries[pml4_index] = VIRT_TO_PHYS(page) | permissions;
        // printk("pml4e final: %p %d\n", pml4->entries[pml4_index], pml4_index);
        // Finally set the new page to pdpt_base
        pdpt_base_addr = (uint64_t)page;
    } else {
        pdpt_base_addr = PHYS_TO_VIRT(pdpt_base_addr);
    }

    // printk("pdpt_index: %d\n", pdpt_index);
    uint64_t pd_base_addr = ((pdpt_t*)pdpt_base_addr)->entries[pdpt_index] & PG_ALIGN;
    // printk("pd_base_addr: %p\n", pd_base_addr);
    // Check to see if we have empty entry
    if(pd_base_addr == 0x0) {
        // Get a new page
        pd_t *page = (pd_t*) PHYS_TO_VIRT(kmalloc_pg());
        // Move the kernel free pointer by 1 page
        kern_free = kern_free + PAGE_SIZE;
        // Zero out the memory
        memset(page, 0, PAGE_SIZE);
        // Set the page into the current index and set permissions in the lower 12 bits
        ((pdpt_t*)pdpt_base_addr)->entries[pdpt_index] = VIRT_TO_PHYS(page) | permissions;
        // printk("pdpte final: %p %d\n", ((pdpt_t*)pdpt_base_addr)->entries[pdpt_index], pdpt_index);
        // Finally set the new page to pd_base
        pd_base_addr = (uint64_t)page;
    } else {
        pd_base_addr = PHYS_TO_VIRT(pd_base_addr);
    }


    // printk("pd_index: %d\n", pd_index);
    uint64_t pt_base_addr = ((pd_t*)pd_base_addr)->entries[pd_index] & PG_ALIGN;
    // printk("pt_base_addr: %p\n", pt_base_addr);
    if(pt_base_addr == 0x0) {
        // Get a new page
        pt_t *page = (pt_t*) PHYS_TO_VIRT(kmalloc_pg());
        // Move the kernel free pointer by 1 page
        kern_free = kern_free + PAGE_SIZE;
        // Zero out the memory
        memset(page, 0, PAGE_SIZE);
        // Set the page into the current index and set permissions in the lower 12 bits
        ((pd_t*)pd_base_addr)->entries[pd_index] = VIRT_TO_PHYS(((uint64_t)page)) | permissions;
        // printk("pte final: %p %d\n", ((pd_t*)pd_base_addr)->entries[pd_index], pd_index);
        // Finally set the new page to pd_base
        pt_base_addr = (uint64_t)page;
    }else {
        pt_base_addr = PHYS_TO_VIRT(pt_base_addr);
    }
    // set_cr3(old_cr3);
    // printk("pte_final_virt: %p\n", pt_base_addr);
    return (pt_t*)pt_base_addr;
}

pt_t* get_pt(pml4_t *pml4, uint64_t virtual_address) {
    uint64_t pml4_index = extract_pml4(virtual_address);
    uint64_t pdpt_index = extract_directory_ptr(virtual_address);
    uint64_t pd_index = extract_directory(virtual_address);
    // Now begin finding the base address for each step of the walk
    // also zero out lower 12 bits for permissions and copy the rest
    //printk("pml4 index: %d\n", pml4_index);
    uint64_t pdpt_base_addr = pml4->entries[pml4_index] & PG_ALIGN;
    //printk("pdpt_base_addr: %p\n", pdpt_base_addr);
    // Check to see if we have empty entry
    if(pdpt_base_addr == 0x0) {
        // Get a new page
        pdpt_t *page = (pdpt_t*) kmalloc_pg();
        // Move the kernel free pointer by 1 page
        kern_free = kern_free + PAGE_SIZE;
        // Zero out the memory
        memset(page, 0, PAGE_SIZE);
        // Set the page into the current index and set permissions in the lower 12 bits
        pml4->entries[pml4_index] = ((uint64_t)page) | USER_SETTINGS;
        //printk("pml4e final: %p\n", pml4->entries[pml4_index]);
        // Finally set the new page to pdpt_base
        pdpt_base_addr = (uint64_t)page;
    }

    //printk("pdpt_index: %d\n", pdpt_index);
    uint64_t pd_base_addr = ((pdpt_t*)pdpt_base_addr)->entries[pdpt_index] & PG_ALIGN;
    //printk("pd_base_addr: %p\n", pd_base_addr);
    // Check to see if we have empty entry
    if(pd_base_addr == 0x0) {
        // Get a new page
        pd_t *page = (pd_t*) kmalloc_pg();
        // Move the kernel free pointer by 1 page
        kern_free = kern_free + PAGE_SIZE;
        // Zero out the memory
        memset(page, 0, PAGE_SIZE);
        // Set the page into the current index and set permissions in the lower 12 bits
        ((pdpt_t*)pdpt_base_addr)->entries[pdpt_index] = ((uint64_t)page) | USER_SETTINGS;
        //printk("pdpte final: %p\n", ((pdpt_t*)pdpt_base_addr)->entries[pdpt_index]);
        // Finally set the new page to pd_base
        pd_base_addr = (uint64_t)page;
    }

    //printk("pd_index: %d\n", pd_index);
    uint64_t pt_base_addr = ((pd_t*)pd_base_addr)->entries[pd_index] & PG_ALIGN;
    //printk("pt_base_addr: %p\n", pt_base_addr);
    if(pt_base_addr == 0x0) {
        // Get a new page
        pt_t *page = (pt_t*) kmalloc_pg();
        // Move the kernel free pointer by 1 page
        kern_free = kern_free + PAGE_SIZE;
        // Zero out the memory
        memset(page, 0, PAGE_SIZE);
        // Set the page into the current index and set permissions in the lower 12 bits
        ((pd_t*)pd_base_addr)->entries[pd_index] = ((uint64_t)page) | USER_SETTINGS;
        //printk("pte final: %p\n", ((pd_t*)pd_base_addr)->entries[pd_index]);
        // Finally set the new page to pd_base
        pt_base_addr = (uint64_t)page;
    }

    return (pt_t*)pt_base_addr;
}

void* kmalloc_pg(void) {
    uint32_t page_index = get_free_page();       // Get a free page from the page allocator
    set_pg_free(page_index, 0);                  // Mark the page in use
    void *address = pg_to_addr(page_index);      // Convert the page index to an address
    return address;
}

bool leaks_pg(uint64_t virt_base, size_t size) {
    uint64_t boundry = (virt_base + PAGE_SIZE) & PG_ALIGN;
    return (virt_base + size) > boundry;
}

void *kmalloc_vma(pml4_t *cr3, uint64_t virt_base, size_t size, uint64_t permissions) {
    void *new_allocation = NULL;
    pml4_t *old_pml4 = get_cr3();
    set_cr3(cr3);
    if(size > 0) {
        pml4_t *old_cr3 = get_cr3();
        set_cr3(cr3);
        // Figure out how many pages we need
        int num_pages = size / PAGE_SIZE;
        num_pages += size % PAGE_SIZE > 0 ? 1 : 0;
        num_pages += leaks_pg(virt_base, size) ? 1 : 0;
        // printk("num_pgs: %d\n", num_pages);
        // Allocate pages and map to virtual address
        for(int i = 0; i < num_pages; i++) {
            uint64_t virt_addr = virt_base + (i * PAGE_SIZE);
            insert_page(cr3, virt_addr, permissions);
            if(i == 0) {
                new_allocation = (void*)virt_addr;
            }
        }
        set_cr3(old_cr3);
    }
    set_cr3(old_pml4);
    printk("new_alloc: %p\n", new_allocation);
    return new_allocation;
}

uint64_t insert_page(pml4_t *cr3, uint64_t virtual_address, uint64_t permissions) {
    // Save the current cr3
    pml4_t *old_cr3 = get_cr3();
    // printk("old: %p new: %p\n", old_cr3, cr3);
    // Set the new cr3
    set_cr3(cr3);
    // Make the pml4 a virtual address
    cr3 = (pml4_t *)PHYS_TO_VIRT(cr3);
    // Get the page table using this pml4 and virtual address
    pt_t* page_table = (pt_t*) get_pt_virt(cr3, virtual_address, permissions);
    // printk("insert_pg_pt: %p\n", page_table);
    // Get a new page to insert into the table
    uint64_t page = (uint64_t)kmalloc_pg() | permissions;
    // printk("new page: %p\n", page);
    // Get the page table offset from the virtual address 
    uint64_t pt_index = extract_table(virtual_address);
    page_table->entries[pt_index] = page;
    // Set back the old cr3
    set_cr3(old_cr3);
    return PHYS_TO_VIRT(page & PG_ALIGN);
}

pml4_t* copy_page_tables(pml4_t *src) {
    pml4_t *copy = NULL;
    if(src != NULL) {
        // printk("kernmem: %p\n", &kernmem);
        // Make src virtual
        src = (pml4_t*)PHYS_TO_VIRT(src);
        // Create a new pml4_t
        copy = (pml4_t*) PHYS_TO_VIRT(kmalloc_pg());
        // Zero out the page
        memset(copy, 0, PAGE_SIZE);
        // Begin copying the first level of the page table
        for(int i = 0; i < MAX_TABLE_ENTRIES; i++) {
            if(src->entries[i] != 0x0) {
                if((src->entries[i] & KERN_SETTINGS) == KERN_SETTINGS) {
                    // Kernel level page, just link it
                    copy->entries[i] = src->entries[i];    
                } else if((src->entries[i] & P) == P) {
                    // Allocate and zero out the new pdpt
                    pdpt_t *new_pml4e = (pdpt_t*) PHYS_TO_VIRT(kmalloc_pg());
                    memset(new_pml4e, 0, PAGE_SIZE);
                    // pdpt entry - Start walking
                    pdpt_t *pml4e = (pdpt_t*) PHYS_TO_VIRT(src->entries[i]);
                    // Start searching the pml4e for present pdpt
                    for(int j = 0; j < MAX_TABLE_ENTRIES; j++) {
                        if((pml4e->entries[j] & P) == P) {
                            // Allocate a new pd and zero out memory
                            pd_t *new_pdpte = (pd_t*) PHYS_TO_VIRT(kmalloc_pg());
                            memset(new_pdpte, 0, PAGE_SIZE);
                            // Start searching the pdpt for present pd
                            pd_t *pdpte = (pd_t *) PHYS_TO_VIRT(pml4e->entries[j]);
                            for(int k = 0; k < MAX_TABLE_ENTRIES; k++) {
                                if((pdpte->entries[k] & P) == P) {
                                    // Allocate a new page table and zero out memory
                                    pt_t *new_pde = (pt_t*) PHYS_TO_VIRT(kmalloc_pg());
                                    memset(new_pde, 0, PAGE_SIZE);
                                    // Start searching the pt for present page
                                    pt_t *pde = (pt_t*) PHYS_TO_VIRT(pdpte->entries[k]);
                                    for(int l = 0; l < MAX_TABLE_ENTRIES; l++) {
                                        if((pde->entries[l] & P) == P) {
                                            // Mark the entry as copy on write 
                                            // and read only
                                            uint64_t address = (pde->entries[l] | COW) & ~RW;
                                            // Add to new page table
                                            new_pde->entries[l] = address;
                                            // Update in old page table
                                            pde->entries[l] = address;
                                        }
                                    }
                                    // Add the pde to the Page directory
                                    new_pdpte->entries[k] = (uint64_t) VIRT_TO_PHYS(new_pde);
                                }
                            }
                            new_pml4e->entries[j] = (uint64_t) VIRT_TO_PHYS(new_pdpte);
                        }
                    }
                    copy->entries[i] = (uint64_t) VIRT_TO_PHYS(new_pml4e);   
                }
            }
        }
        // Convert the new cr3 into a physical address
        copy = (pml4_t*) VIRT_TO_PHYS(copy);
    }
    return copy;
}

void kfree_pg(void *address) {
    uint32_t page_index = addr_to_pg((void*)VIRT_TO_PHYS(address));
    set_pg_free(page_index, 1);
}


void dump_tables(pml4_t *cr3) {
    if(cr3 != NULL) {
        pml4_t *old_cr3 = get_cr3();
        set_cr3(cr3);
        printk("cr3: %p\n", cr3);
        // Make cr3 virtual
        cr3 = (pml4_t*)PHYS_TO_VIRT(cr3);
        for(int i = 0; i < 512; i++) {
            if(cr3->entries[i] != 0x0) {
                printk("index: %d pml4e: %p\n", i, cr3->entries[i]);
                pdpt_t *pml4e = (pdpt_t*)PHYS_TO_VIRT(cr3->entries[i] & PG_ALIGN);
                for(int j = 0; j < 512; j++) {
                    if(pml4e->entries[j] != 0x0){
                        printk("index: %d pdpte: %p\n", j, pml4e->entries[j]);
                        pd_t *pdpte = (pd_t*)PHYS_TO_VIRT(pml4e->entries[j] & PG_ALIGN);
                        for(int k = 0; k < 512; k++) {
                            if(pdpte->entries[k] != 0x0) {
                                printk("index: %d pde: %p\n", k, pdpte->entries[k]);
                                pt_t *pde = (pt_t*)PHYS_TO_VIRT(pdpte->entries[k] & PG_ALIGN);
                                for(int l = 0; l < 512; l++) {
                                    if(pde->entries[l] != 0x0) {
                                        printk("index: %d pte: %p\n", l, pde->entries[l]);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        set_cr3(old_cr3);
    }
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

void set_cr3(pml4_t *pml4) {
    __asm__ __volatile__("movq %0, %%cr3;"
                         :
                         : "r"(pml4)
                         :
                         );
}

pml4_t* get_cr3(void) {
    pml4_t *pml4 = NULL;
    __asm__ __volatile__("movq %%cr3,  %0;"
                         : "=r"(pml4)
                         :
                         :
                         );
    return pml4;
}
