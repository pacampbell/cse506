#define __KERNEL__
#include <sys/pgtable.h>
#include <sbunix/debug.h>

extern char kernmem;
extern void *kern_free;
extern void *kern_base;
extern pml4_t *kernel_cr3;

pml4_t *g_kernel_pgtable = NULL;
uint32_t *free_pg_list;
uint64_t free_pg_list_end;
uint64_t free_pg_offset;

void* pg_to_addr(uint64_t pg) {
    //return (void*)((pg * PAGE_SIZE) + ((uint64_t)free_pg_list_end));
    //return (void*)((pg * PAGE_SIZE) + ((uint64_t)kern_base));
    return (void*)((pg * PAGE_SIZE) + (free_pg_offset));
    
}

int addr_to_pg(void* addr) {
    //return ((uint64_t)addr - (uint64_t)free_pg_list_end) / PAGE_SIZE;
    //return ((uint64_t)addr - (uint64_t)kern_base) / PAGE_SIZE;
    // printk("free_pg_offset: %p, PAGE_SIZE: %p, addr: %p\n", free_pg_offset, PAGE_SIZE, addr);
    return ((uint64_t)addr - free_pg_offset) / PAGE_SIZE;
}

int get_free_page() {
    int pg = 0;

    for(; pg < MAX_PAGES && !is_pg_free(pg); pg++);

    if(pg >= MAX_PAGES) {
        panic("Ran out of pages!!!!\n");
        halt();
        return -1;
    }

    return pg;
}

void mark_address_range_free(uint64_t start, uint64_t end, int free) {
    if(end < free_pg_offset) {
        // panic("skipping\n");
        return;
    }
    if(start < free_pg_offset) start = free_pg_offset;
    int s, e; 
    s = addr_to_pg((void*)start);
    e = addr_to_pg((void*)end);

    for (; s <= e; s++) {
        set_pg_free(s, free);
    }
}

void mem_usg() {
    uint64_t used = 0, free = 0;
    for (int i = 0; i < MAX_PAGES; i++) {
        if(is_pg_free(i)) {
            free++;
        } else {
            used++;
        }       
    }

    printk("free: %d, used: %d, %%%d", free, used, free/used);

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
        *(free_pg_list + i) &= ~all_on;
    }

    free_pg_list_end = (uint64_t)(free_pg_list + size);

    kern_free = (void*)PG_RND_UP(free_pg_list_end);
    printk("start: %p, end: %p, kern_free: %p\n", free_pg_list, free_pg_list_end, kern_free);
    free_pg_offset = (uint64_t)kern_free;
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
    panic("STOP USING ME!!\n");
    halt();
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
    pml4->entries[510] = (uint64_t)pml4 | P | RW | US;
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
    // printk("pml4-selfref: %p\n", pml4_selfref);
    // printk("pdpt-selfref: %p\n", pdpt_selfref);
    // printk("Virtual address: %p\n", virtual_address);
    uint64_t pml4e = get_pml4e(pml4, virtual_address);
    if(pml4e == 0) {
       pml4e = insert_pml4e(pml4, virtual_address, (uint64_t)kmalloc_pg() | permissions); 
    }
    // printk("pml4e: %p entry: %d\n", pml4e, extract_pml4(virtual_address));

    uint64_t pdpte = get_pdpte(pml4, virtual_address);
    if(pdpte == 0) {
        pdpte = insert_pdpte(pml4, virtual_address, (uint64_t)kmalloc_pg() | permissions);
    }
    // printk("pdpte: %p entry: %d\n", pdpte, extract_directory_ptr(virtual_address));

    uint64_t pde = get_pde(pml4, virtual_address);
    if(pde == 0) {
        pde = insert_pde(pml4, virtual_address, (uint64_t)kmalloc_pg() | permissions);
    }
    // printk("pde: %p entry: %d\n", pde, extract_directory(virtual_address));
    return (pt_t*)PHYS_TO_VIRT(pde);
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
        // printk("pml4e final: %p\n", pml4->entries[pml4_index]);
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
        printk("pdpte final: %p\n", ((pdpt_t*)pdpt_base_addr)->entries[pdpt_index]);
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
        printk("pte final: %p\n", ((pd_t*)pd_base_addr)->entries[pd_index]);
        // Finally set the new page to pd_base
        pt_base_addr = (uint64_t)page;
    }

    return (pt_t*)pt_base_addr;
}

void* kmalloc_pg(void) {
    uint32_t page_index = get_free_page();       // Get a free page from the page allocator
    set_pg_free(page_index, USED);                  // Mark the page in use
    void *address = pg_to_addr(page_index);      // Convert the page index to an address
    // printk("new pg: %p\n", address);
    return address;
}

void *kmalloc_kern(size_t size) {
    uint32_t page_index = get_free_page();
    void *address = (void*)PHYS_TO_VIRT(pg_to_addr(page_index));
    return kmalloc_vma(kernel_cr3, (uint64_t)address, size, KERN_SETTINGS);
}

bool leaks_pg(uint64_t virt_base, size_t size) {
    uint64_t boundry = (virt_base + PAGE_SIZE) & PG_ALIGN;
    return (virt_base + size) > boundry;
}

void *kmalloc_vma(pml4_t *cr3, uint64_t virt_base, size_t size, uint64_t permissions) {
    void *new_allocation = NULL;
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
    get_pt_virt(cr3, virtual_address, permissions);
    // Get a new page to insert into the table
    uint64_t pg_tmp = (uint64_t)kmalloc_pg();
    // printk("new pg: %p\n", pg_tmp);
    uint64_t page = pg_tmp | permissions;
    uint64_t exists = get_pte(cr3, virtual_address);
    if(exists) {
        insert_pte(cr3, virtual_address, page);
        FLUSH_TLB(virtual_address); 
    } else {
        insert_pte(cr3, virtual_address, page);
    }
    return PHYS_TO_VIRT(page & PG_ALIGN);
}

pml4_t* copy_page_tables(Task *src) {
    pml4_t *copy = NULL;
    if(src != NULL) {
        pml4_t *src_pml4 = (pml4_t*)src->registers.cr3;
        // pml4_t *ocr3 = get_cr3();
        set_cr3(src_pml4);
        // printk("kernel: %p ocr3: %p src: %p\n", kernel_cr3, ocr3, src);
        src_pml4  =(pml4_t*) PHYS_TO_VIRT(src_pml4);
        copy = (pml4_t*) kmalloc_kern(PAGE_SIZE);
        pml4_t *copy_phys = (pml4_t*) VIRT_TO_PHYS(copy);
        pml4_t *src_phys = (pml4_t*) VIRT_TO_PHYS(src_pml4);
        // Zero out the new pml4
        memset(copy, 0, sizeof(pml4_t));
        // Link the kernel pages into this page table entry
        copy->entries[511] = src_pml4->entries[511];
        set_cr3(copy_phys);
        // printk("kernel: %p ocr3: %p src: %p current: %p\n", kernel_cr3, ocr3, src, get_cr3());
        copy->entries[510] = (uint64_t)copy_phys | P | RW | US;
        set_cr3(src_phys);
        printk("kernel: %p src: %p cpy: %p\n", kernel_cr3, src_phys, copy_phys);
        // Look for user space pages
        /*
        for(int pml4_index = 0; pml4_index < MAX_TABLE_ENTRIES - 2; pml4_index++) {
            if((src->entries[pml4_index] & P) == P) {
                // uint64_t pml4sr = 0xff7fbfdfe000 | pml4_index;
                set_cr3(copy_phys);
                copy->entries[pml4_index] = src->entries[pml4_index]; 
                // uint64_t pml4e_phys = (uint64_t)kmalloc_pg() | USER_SETTINGS;
                // copy->entries[pml4_index] = pml4e_phys;
                // insert_pml4e(copy_phys, pml4sr, pml4e_phys);
                // Get the src entry to search through next
                // set_cr3(src_phys);
                // pdpt_t *pml4e = (pdpt_t*)(PHYS_TO_VIRT(src->entries[pml4_index] & PG_ALIGN));
                // printk("current cr3: %p\n", get_cr3());
                // printk("pdpte: %p\n", pml4e->entries[0]);
                // halt();
            }
        }
        set_cr3(ocr3);
        */
        // Change the copy back to a physical address
        copy = copy_phys;
    }
    return copy;
}

void check_vma_permissions(pml4_t *cr3, uint64_t address) {
    if(cr3 != NULL) {
        pdpt_t *pdpt = (pdpt_t*) get_pml4e(cr3, address);
        if(pdpt != 0x0) {
            printk("pdpt: %p\n", pdpt);
            pd_t *pd = (pd_t*)get_pdpte(cr3, address);
            if(pd != 0x0) {
                printk("pd: %p\n", pd);
                pt_t *pt = (pt_t*)get_pde(cr3, address);
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
            if(i != 511 && i != 510 && cr3->entries[i] != 0x0) {
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
                                printk("pde: %p\n", pde->entries[0]);
                                halt();
                                for(int l = 0; l < 512; l++) {
                                    if(pde->entries[l] != 0x0) {
                                        printk("index: %d pte: %p\n", l, pde->entries[l]);
                                        // BOCHS_MAGIC();
                                    }
                                }
                            }
                        }
                    }
                }
            } else if(i == 511){
                printk("Kernel page link [%d]\n", i);
            } else if(i == 510){
                printk("Self reference entry[%d]\n", i);
            }
        }
        set_cr3(old_cr3);
    }
}

uint64_t insert_pml4e(pml4_t *cr3, uint64_t virtual_address, uint64_t frame) {
    // Save previous cr3
    pml4_t *ocr3 = get_cr3();
    // Set the new cr3
    set_cr3(cr3);
    PML4T_ADDR[extract_pml4(virtual_address)] = frame;
    // Set back the previous cr3
    set_cr3(ocr3);
    return frame;
}

uint64_t get_pml4e(pml4_t *cr3, uint64_t virtual_address) {
    // Save previous cr3
    pml4_t *ocr3 = get_cr3();
    // Set the new cr3
    set_cr3(cr3);
    //printk("get_pml4e: kernel_cr3: %p current cr3: %p virtual: %p\n", kernel_cr3, get_cr3(), virtual_address);
    // Get the value at the address
    uint64_t phys_addr = PML4T_ADDR[extract_pml4(virtual_address)];
    // Set back the previous cr3
    set_cr3(ocr3);
    return phys_addr;
}

uint64_t get_pdpte(pml4_t *cr3, uint64_t virtual_address) {
    // Save previous cr3
    pml4_t *ocr3 = get_cr3();
    // Set the new cr3
    set_cr3(cr3);
    //printk("get_pdpte: kernel_cr3: %p current cr3: %p virtual: %p\n", kernel_cr3, get_cr3(), virtual_address);
    // Get the value at the address
    uint64_t *base_addr = (uint64_t*)((uint64_t)PDPT_ADDR | (extract_pml4(virtual_address) << 12));
    uint64_t phys_addr = base_addr[extract_directory_ptr(virtual_address)];
    // Set back the previous cr3
    set_cr3(ocr3);
    return phys_addr;
}

uint64_t insert_pdpte(pml4_t *cr3, uint64_t virtual_address, uint64_t frame) {
    // Save previous cr3
    pml4_t *ocr3 = get_cr3();
    // Set the new cr3
    set_cr3(cr3);
    // printk("get cr3: %p\n", get_cr3());
    // Get the value at the address
    uint64_t *base_addr = (uint64_t*)((uint64_t)PDPT_ADDR | (extract_pml4(virtual_address) << 12));
    base_addr[extract_directory_ptr(virtual_address)] = frame;
    // Set back the previous cr3
    set_cr3(ocr3);
    return frame;
}


uint64_t get_pde(pml4_t *cr3, uint64_t virtual_address) {
    // Save previous cr3
    pml4_t *ocr3 = get_cr3();
    // Set the new cr3
    set_cr3(cr3);
    //printk("get_pde: kernel_cr3: %p current cr3: %p virtual: %p\n", kernel_cr3, get_cr3(), virtual_address);
    // Get the value at the address
    uint64_t *base_addr = (uint64_t*)((uint64_t)PGDIR_ADDR | (extract_pml4(virtual_address) << 21) | (extract_directory_ptr(virtual_address) << 12));
    uint64_t phys_addr = base_addr[extract_directory(virtual_address)];
    // Set back the previous cr3
    set_cr3(ocr3);
    return phys_addr;
}

uint64_t insert_pde(pml4_t *cr3, uint64_t virtual_address, uint64_t frame) {
    // Save previous cr3
    pml4_t *ocr3 = get_cr3();
    // Set the new cr3
    set_cr3(cr3);
    // Get the value at the address
    uint64_t *base_addr = (uint64_t*)((uint64_t)PGDIR_ADDR | (extract_pml4(virtual_address) << 21) | (extract_directory_ptr(virtual_address) << 12));
    base_addr[extract_directory(virtual_address)] = frame;
    // Set back the previous cr3
    set_cr3(ocr3);
    return frame;
}

uint64_t get_pte(pml4_t *cr3, uint64_t virtual_address) {
    // Save previous cr3
    pml4_t *ocr3 = get_cr3();
    // Set the new cr3
    set_cr3(cr3);
    //printk("get_pte: kernel_cr3: %p current cr3: %p virtual: %p\n", kernel_cr3, get_cr3(), virtual_address);
    // Get the value at the address
    uint64_t *base_addr = (uint64_t*)((uint64_t)PGTBL_ADDR | (extract_pml4(virtual_address) << 30) | (extract_directory_ptr(virtual_address) << 21) | (extract_directory(virtual_address) << 12));
    uint64_t phys_addr = base_addr[extract_table(virtual_address)];
    // Set back the previous cr3
    set_cr3(ocr3);
    return phys_addr;
}

uint64_t insert_pte(pml4_t *cr3, uint64_t virtual_address, uint64_t frame) {
    // Save previous cr3
    pml4_t *ocr3 = get_cr3();
    // Set the new cr3
    set_cr3(cr3);
    uint64_t *base_addr = (uint64_t*)((uint64_t)PGTBL_ADDR | (extract_pml4(virtual_address) << 30) | (extract_directory_ptr(virtual_address) << 21) | (extract_directory(virtual_address) << 12));
    base_addr[extract_table(virtual_address)] = frame;
    // Set back the previous cr3
    set_cr3(ocr3);
    return frame;
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
