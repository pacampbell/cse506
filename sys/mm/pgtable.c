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
        // printk("pdpte final: %p\n", ((pdpt_t*)pdpt_base_addr)->entries[pdpt_index]);
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
        // printk("pte final: %p\n", ((pd_t*)pd_base_addr)->entries[pd_index]);
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
            // printk("virt_addr: %p\n", virt_addr);
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

pml4_t* copy_page_tables(pml4_t *src, struct mm_struct *mm) {
    pml4_t *copy = NULL;
    if(src != NULL) {
        pml4_t *ocr3 = get_cr3();
        // printk("kernel: %p ocr3: %p src: %p\n", kernel_cr3, ocr3, src);
        src  = (pml4_t*) PHYS_TO_VIRT(src);
        copy = (pml4_t*) kmalloc_kern(PAGE_SIZE);
        pml4_t *copy_phys = (pml4_t*)VIRT_TO_PHYS(copy);
        pml4_t *src_phys = (pml4_t*)VIRT_TO_PHYS(src);
        // Zero out the new pml4
        memset(copy, 0, sizeof(pml4_t));
        // Link the kernel pages into this page table entry
        copy->entries[511] = src->entries[511];
        set_cr3(copy_phys);
        // printk("kernel: %p ocr3: %p src: %p current: %p\n", kernel_cr3, ocr3, src, get_cr3());
        copy->entries[510] = (uint64_t)copy_phys | P | RW | US;
        // Go back to the source page tables and start searching for user pages
        set_cr3(src_phys);
        
        if(mm != NULL) {
            for(struct vm_area_struct *vma = mm->mmap; vma != NULL; vma = vma->next) {
                // printk("vma: %p - %s\n", vma->vm_start, (vma->vm_prot & VM_GROWSDOWN) == VM_GROWSDOWN ? "GROWS DOWN" : "GROWS UP");
                if(get_pml4e(src_phys, USER_VIRT_OFFSET) != 0x0) {
                    char *buffer = kmalloc_kern(PAGE_SIZE);
                    uint64_t address = vma->vm_start & PG_ALIGN;
                    uint64_t pte;
                    uint64_t grow_amount = (vma->vm_prot & VM_GROWSDOWN) == VM_GROWSDOWN ? -PAGE_SIZE : PAGE_SIZE;
                    while((pte = get_pte(src_phys, address)) != 0x0) {
                        // Create the pt if it does not exist
                        get_pt_virt(copy_phys, address, USER_SETTINGS);
                        // Create a new pte in the copy page tables
                        insert_pte(copy_phys, address, (uint64_t)kmalloc_pg() | USER_SETTINGS);
                        // Copy the src pte contents into a buffer
                        set_cr3(src_phys);
                        memcpy(buffer, (void*)address, PAGE_SIZE);
                        // Copy the buffer into new pte just inserted in the copy table
                        set_cr3(copy_phys);
                        memcpy((void*)address, buffer, PAGE_SIZE);
                        // Set back to the src page tables
                        set_cr3(src_phys);
                        // Increment the address by 1 page and try again
                        address += grow_amount;
                    }
                    // release the page content buffer
                    kfree_pg(buffer);
                }
            }
        }
        // Change the copy back to a physical address
        copy = copy_phys;
        set_cr3(ocr3);
    }
    return copy;
}

void check_vma_permissions(pml4_t *cr3, uint64_t address) {
    // if(cr3 != NULL) {
    //     pdpt_t *pdpt = get_pml4e(cr3, address);
    //     if(pdpt != 0x0) {
    //         printk("pdpt: %p\n", pdpt);
    //         pd_t *pd = get_pdpte(cr3, address);
    //         if(pd != 0x0) {
    //             printk("pd: %p\n", pd);
    //             pt_t *pt = get_pde(cr3, address);
    //             if(pt != 0x0) {
    //                 printk("pt: %p\n", pt);
    //                 uint64_t page = get_pte(cr3, address);
    //                 if(page != 0x0) {
    //                     printk("page: %p\n", page);
    //                 } else {
    //                     printk("No page entry for vma %p\n", address);
    //                 }
    //             } else {
    //                 printk("No page table exists for vma %p\n", address);
    //             }
    //         } else {
    //             printk("No page directory exists for vma %p\n", address);
    //         }
    //     } else {
    //         printk("No page directory pointer table exists for vma %p\n", address);
    //     }
    // }
}

void kfree_pg(void *address) {
    uint32_t page_index = addr_to_pg((void*)VIRT_TO_PHYS(address));
    set_pg_free(page_index, 1);
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

/*
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
        printk("cr3: %p virtual address; %p\n", cr3, virtual_address);
        uint64_t pdpte_index = extract_directory_ptr(virtual_address);
        // Save current cr3
        pml4_t *current_cr3 = get_cr3();
        // set new cr3
        set_cr3(cr3);
        // Get the pdpt
        pdpt_t *pdpt = (pdpt_t*)get_pml4e(cr3, virtual_address);
        if(pdpt != 0x0) {
            printk("pdpt: %p\n", pdpt);
            panic("BEFORE MOTHER FUCKING CRASH\n");
            pdpt = (pdpt_t*)PHYS_TO_VIRT((uint64_t)pdpt & PG_ALIGN);
            printk("dskjhfsdkjfksjd pdpt: %p\n", pdpt);
            printk("Got it: pml4e %p\n", pdpt->entries[0]);
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
*/

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
            printk("No page directory pointer entry for this virtual address %p\n", virtual_address);
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
            printk("No existing page directory for this virtual address %p\n", virtual_address);
        }
        // Save the current cr3
        pml4_t *current_cr3 = get_cr3();
        // Set the new cr3
        set_cr3(cr3);
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
