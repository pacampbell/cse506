#define __KERNEL__
#include <sys/pgtable.h>
#include <sbunix/debug.h>

extern char kernmem;
extern void *kern_free;
extern void *kern_base;



pml4_t *g_kernel_pgtable = NULL;
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
    // Store the kernels global page table value
    g_kernel_pgtable = pml4;
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
    pml4_t *old_cr3 = get_cr3();
    set_cr3(pml4);
    // printk("virtual_address: %p\n", virtual_address);
    // Now begin finding the base address for each step of the walk
    // also zero out lower 12 bits for permissions and copy the rest
    // printk("pml4 index: %d\n", pml4_index);
    pml4 = (pml4_t*)PHYS_TO_VIRT((uint64_t)pml4 & PG_ALIGN);
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
    set_cr3(old_cr3);
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
        pml4->entries[pml4_index] = ((uint64_t)page) | KERN_SETTINGS;
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
        ((pdpt_t*)pdpt_base_addr)->entries[pdpt_index] = ((uint64_t)page) | KERN_SETTINGS;
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
        ((pd_t*)pd_base_addr)->entries[pd_index] = ((uint64_t)page) | KERN_SETTINGS;
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
    // pml4_t *old_pml4 = get_cr3();
    // set_cr3(cr3);
    if(size > 0) {
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
    }
    // set_cr3(old_pml4);
    return new_allocation;
}

uint64_t calculate_total_page_by_size(size_t size, uint64_t base_addr) {
    uint64_t num_pages = size / PAGE_SIZE;
    num_pages += size % PAGE_SIZE > 0 ? 1 : 0;
    num_pages += leaks_pg(base_addr, size) ? 1 : 0;
    return num_pages;
}

uint64_t insert_page(pml4_t *cr3, uint64_t virtual_address, uint64_t permissions) {
    // Get the page table using this pml4 and virtual address
    pt_t* page_table = (pt_t*) get_pt_virt(cr3, virtual_address, permissions);
    pml4_t *old_cr3 = get_cr3();
    set_cr3(cr3);
    // Get a new page to insert into the table
    uint64_t page = (uint64_t)kmalloc_pg() | permissions;
    // Get the page table offset from the virtual address 
    uint64_t pt_index = extract_table(virtual_address);
    if(page_table->entries[pt_index] != 0x0) {
        // panic("PAGE ALREADY EXISTS\n");
        // printk("Old Entry: %p\n", page_table->entries[pt_index]);
        // Page Already exists so insert then invalidate the tlb
        page_table->entries[pt_index] = page;
        // printk("New Entry: %p\n", page_table->entries[pt_index]);
        FLUSH_TLB(virtual_address);
    } else {
        page_table->entries[pt_index] = page;
    }
    // Set back the old cr3
    set_cr3(old_cr3);
    return PHYS_TO_VIRT(page & PG_ALIGN);
}

extern pml4_t *kernel_cr3;
pml4_t* copy_page_tables(pml4_t *src) {
    pml4_t *copy = NULL;

    if(src != NULL) {
        // panic("START COPYING PAGE TABLES\n");
        pml4_t *cr3_current = get_cr3();
        // Convert the source to virtual
        src  = (pml4_t*) PHYS_TO_VIRT(src);
        // Make new page for copy pml
        copy = (pml4_t*) PHYS_TO_VIRT(kmalloc_pg());
        memset(copy, 0, sizeof(pml4_t));
        // Start interating through the pml4e's
        for(int i = 0; i < MAX_TABLE_ENTRIES; i++) {
            if((src->entries[i] & USER_SETTINGS) == 0x3) {
                // printk("Linking Kernel entry[%d]: %p\n", i, src->entries[i]);
                copy->entries[i] = src->entries[i];
            } else if((src->entries[i] & P) == P) {
                // printk("Cloning user entry[%d]: %p\n", i, src->entries[i]);
                // For every user pml4e we need a new page
                pdpt_t *pml4e = (pdpt_t*)((uint64_t)kmalloc_pg() | USER_SETTINGS);
                // Add this entry to the pml4
                copy->entries[i] = (uint64_t)pml4e;
                // Now make it virtual
                pml4e = (pdpt_t*)PHYS_TO_VIRT((uint64_t)pml4e & PG_ALIGN);
                memset(pml4e, 0, sizeof(pdpt_t));
                // Begin iterating through the pdpte's
                pdpt_t *src_pml4e = (pdpt_t*)PHYS_TO_VIRT(src->entries[i] & PG_ALIGN);
                for(int j = 0; j < MAX_TABLE_ENTRIES; j++) {
                    if((src_pml4e->entries[j] & P) == P) {
                        // printk("Cloning page directory pointer table entry[%d]: %p\n", j, src_pml4e->entries[j]);
                        // for every pdpte we need a new page
                        pd_t *pdpte = (pd_t*)((uint64_t)kmalloc_pg() | USER_SETTINGS);
                        pml4e->entries[j] = (uint64_t)pdpte;
                        // now make it virtual
                        pdpte = (pd_t*)PHYS_TO_VIRT((uint64_t)pdpte & PG_ALIGN);
                        memset(pdpte, 0, sizeof(pd_t));
                        // Begin interating though the pde's
                        pd_t *src_pdpte = (pd_t*)PHYS_TO_VIRT(src_pml4e->entries[j] & PG_ALIGN);
                        for(int k = 0; k < MAX_TABLE_ENTRIES; k++) {
                            if((src_pdpte->entries[k] & P) == P) {
                                // printk("Cloning page directory entry[%d]: %p\n", k, src_pdpte->entries[k]);
                                // For every pte we need a new page
                                pt_t *pde = (pt_t*)((uint64_t)kmalloc_pg() | USER_SETTINGS);
                                pdpte->entries[k] = (uint64_t)pde;
                                // Now make it virtual
                                pde = (pt_t*)PHYS_TO_VIRT((uint64_t)pde & PG_ALIGN);
                                memset(pde, 0, sizeof(pt_t));
                                // begin interaying through all the pte's
                                pt_t *src_pde = (pt_t*)PHYS_TO_VIRT(src_pdpte->entries[k] & PG_ALIGN);
                                for(int l = 0; l < MAX_TABLE_ENTRIES; l++) {
                                    if((src_pde->entries[l] & P) == P) {
                                        // We found a page!
                                        // Not only do we need to make a new page
                                        // We need to copy the contents of said page
                                        // Make a temp buffer
                                        char page_buff[PAGE_SIZE];
                                        // Get a new page
                                        // set_cr3(phys_cpy_cr3);
                                        uint64_t page = (uint64_t)kmalloc_pg() | USER_SETTINGS; 
                                        pde->entries[l] = page;
                                        // Set no cr3 to copy contents
                                        // set_cr3(src);
                                        // Copy the page
                                        memcpy(page_buff, (void*)PHYS_TO_VIRT(src_pde->entries[l] & PG_ALIGN), PAGE_SIZE);
                                        // Change cr3
                                        // set_cr3(phys_cpy_cr3);
                                        memcpy((void*)PHYS_TO_VIRT(page & PG_ALIGN), page_buff, PAGE_SIZE);
                                        // set_cr3(phys_src_cr3);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        // Change the copy back to a physical address
        copy = (pml4_t*)VIRT_TO_PHYS(copy);
        // Set back the original page tables
        set_cr3(cr3_current);
    }
    // panic("DONE COPYING PAGE TABLES\n");
    return copy;
}

void check_vma_permissions(pml4_t *cr3, uint64_t address) {
    if(cr3 != NULL) {
        pdpt_t *pdpt = get_pml4e(cr3, address);
        if(pdpt != 0x0) {
            printk("pdpt: %p\n", pdpt);
            pd_t *pd = get_pdpte(cr3, address);
            if(pd != 0x0) {
                printk("pd: %p\n", pd);
                pt_t *pt = get_pde(cr3, address);
                if(pt != 0x0) {
                    printk("pt: %p\n", pt);
                    uint64_t page = get_pte(cr3, address);
                    if(page != 0x0) {
                        printk("page: %p\n", page);
                    } else {
                        printk("No page entry for vma %p\n", address);
                    }
                } else {
                    printk("No page table exists for vma %p\n", address);
                }
            } else {
                printk("No page directory exists for vma %p\n", address);
            }
        } else {
            printk("No page directory pointer table exists for vma %p\n", address);
        }
    }
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
                continue;
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

pdpt_t* get_pml4e(pml4_t *cr3, uint64_t virtual_address) {
    pdpt_t *pml4e = NULL;
    if(cr3 != NULL) {
        uint64_t pml4e_index = extract_pml4(virtual_address);
        // Save current cr3
        pml4_t *current_cr3 = get_cr3();
        // set new cr3
        set_cr3(cr3);
        // Get the pml4_e
        cr3 = (pml4_t*)PHYS_TO_VIRT((uint64_t)cr3 & PG_ALIGN);
        pml4e = (pdpt_t*)(cr3->entries[pml4e_index]);
        // restore old cr3
        set_cr3(current_cr3);
    }
    return pml4e;
}

pd_t* get_pdpte(pml4_t *cr3, uint64_t virtual_address) {
    pd_t *pdpte = NULL;
    if(cr3 != NULL) {
        uint64_t pdpte_index = extract_directory_ptr(virtual_address);
        // Save current cr3
        pml4_t *current_cr3 = get_cr3();
        // set new cr3
        set_cr3(cr3);
        // Get the pdpt
        pdpt_t *pdpt = (pdpt_t*)get_pml4e(cr3, virtual_address);
        if(pdpt != 0x0) {
            pdpt = (pdpt_t*)PHYS_TO_VIRT((uint64_t)pdpt & PG_ALIGN);
            // Get the pdpte
            pdpte = (pd_t*)(pdpt->entries[pdpte_index]);
        }
        // restore old cr3
        set_cr3(current_cr3);
    }
    return pdpte;
}

pt_t* get_pde(pml4_t *cr3, uint64_t virtual_address) {
    pt_t *pde = NULL;
    if(cr3 != NULL) {
        uint64_t pde_index = extract_directory(virtual_address);
        // Save current cr3
        pml4_t *current_cr3 = get_cr3();
        // set new cr3
        set_cr3(cr3);
        // get the page directory
        pd_t *pd = (pd_t*)get_pdpte(cr3, virtual_address);
        if(pd != 0x0) {
            pd = (pd_t*)PHYS_TO_VIRT((uint64_t)pd & PG_ALIGN);
            // get the pde
            pde = (pt_t*)pd->entries[pde_index];
        }
        // restore old cr3
        set_cr3(current_cr3); 
    }
    return pde;
}

uint64_t get_pte(pml4_t *cr3, uint64_t virtual_address) {
    uint64_t pg = 0;
    if(cr3 != NULL) {
        uint64_t pte_index = extract_table(virtual_address);
        // Save current cr3
        pml4_t *current_cr3 = get_cr3();
        // set new cr3
        set_cr3(cr3);
        pt_t *pt = (pt_t*)get_pde(cr3, virtual_address);
        if(pt != 0x0) {
            pt = (pt_t*)PHYS_TO_VIRT((uint64_t)pt & PG_ALIGN);
            // Get the actual page entry address
            pg = pt->entries[pte_index];
        }
        // restore old cr3
        set_cr3(current_cr3); 
    }
    return pg;
}

/*
pdpt_t* insert_pml4e(pml4_t *cr3, uint64_t virtual_address) {
    pdpt_t *pml4e = NULL; 
    if(cr3 != NULL) {
        // Extract the index from the virtual addresss
        uint64_t pml4e_index = extract_pml4(virtual_address);
        // Save the current cr3
        pml4_t *current_cr3 = get_cr3();
        // Set new cr3
        set_cr3(cr3);
        // Check to see if there is already an entry for this virtual address
        pml4e = get_pml4e(cr3, virtual_address);
        // If its not null we need to flush
        if(pml4e != NULL) {
            printk("Overwriting pml4e entry @ %d in cr3 %p\n", pml4e_index, cr3);
            // FLUSH_TLB(virtual_address);
        } else {
            printk("Found existing page directory pointer table %p\n", pml4e);
        }
        // Get a new page for the pdpt
        uint64_t pdpt = (uint64_t)kmalloc_pg() | USER_SETTINGS;
        // Overwrite/add the entry
        cr3 = (pml4_t*)PHYS_TO_VIRT(((uint64_t)cr3) & PG_ALIGN);
        cr3->entries[pml4e_index] = pdpt;
        // Send back the physical address to be used
        pml4e = (pdpt_t*)pdpt;
        // Set the cr3 to the correct address
        set_cr3(current_cr3);
    }
    return pml4e;
}

pd_t* insert_pdpte(pml4_t *cr3, uint64_t virtual_address) {
    pd_t *pdpte = NULL;
    if(cr3 != NULL) {
        // Extract the index from the virtual address
        uint64_t pdpte_index = extract_directory_ptr(virtual_address);
        // Save the current cr3
        pml4_t *current_cr3 = get_cr3();
        // Set the new cr3
        set_cr3(cr3);
        pdpt_t *pdpt = get_pml4e(cr3, virtual_address);
        if(pdpt == NULL) {
            pdpt = insert_pml4e(cr3, virtual_address);
        } else {
            printk("Found existing page directory pointer table %p\n", pdpt);
        }
        // Check to see if there is already an entry for the virtual address
        pdpte = get_pdpte(cr3, virtual_address);
        if(pdpte != NULL) {
             printk("Overwriting pdpte entry @ %d in cr3 %p\n", pdpte_index, cr3);
             // FLUSH_TLB(virtual_address);
        } else {
            printk("Found existing page directory %p\n", pdpte);
        }
        // Get a new page for the pdpte
        uint64_t pd = (uint64_t)kmalloc_pg() | USER_SETTINGS;
        // Overwirte/add the entry
        pdpt = (pdpt_t*)PHYS_TO_VIRT((uint64_t)pdpt & PG_ALIGN);
        pdpt->entries[pdpte_index] = pd;
        // Send back the physical address to be used
        pdpte = (pd_t*)pd;
        // Restore the current cr3
        set_cr3(current_cr3);
    }
    return pdpte;
}

pt_t* insert_pde(pml4_t *cr3, uint64_t virtual_address) {
    pt_t *pde = NULL;
    if(cr3 != NULL) {
        // Extract the index from the virtual address
        // uint64_t pde_index = extract_directory(virtual_address);
        // Save the current cr3
        // pml4_t *current_cr3 = get_cr3();
        // Set the new cr3
        // set_cr3(cr3);

        // Restore the current cr3
        // set_cr3(current_cr3);
    }
    return pde;
}

uint64_t insert_pte(pml4_t *cr3, uint64_t virtual_address, uint64_t permissions) {
    return 0;
}
*/

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
