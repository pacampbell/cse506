#define __KERNEL__
#include <sys/elf.h>
#include <sbunix/string.h>
#include <sys/mm/vma.h>
#include <sys/pgtable.h>
#include <sys/screen.h>

void print_elf_hdr(Elf64_Ehdr *hdr);

void load_elf(char *data, uint64_t length, struct pml4_t *new_pml4) {
    if(validate_header(data)) {
        printk("Valid ELF header for system.\n");
        Elf64_Ehdr *hdr = (Elf64_Ehdr*)data;

        Elf64_Shdr *section = (Elf64_Shdr*)(hdr->e_shoff + (char*)data);

        if(hdr->e_shstrndx == 0x00) panic("NO STRING TABLE");
        char* str_tab = ((char*)data + section[hdr->e_shstrndx].sh_offset);
        //char* str_tab = (char*)(section[(hdr->e_shstrndx )]);
        int num_secs = hdr->e_shnum;
        char *name;
        int txt = 0, rodata = 0, data = 0, bss = 0;
        for(int i = 0; i < num_secs; i++) {
            name = &str_tab[section[i].sh_name];
            printk("name: %s\n", name);

            if(strcmp(".text", name)) {
                //txt = section[i].sh_offset;
                txt = i;
                printk("found .txt at: %p\n", txt);

            } else if(strcmp(".rodata", name)) {
                //rodata = section[i].sh_addr;
                rodata = i;
                //printk("found .rodata at: %p\n",rodata);

            } else if(strcmp(".data", name)) {
                //data = section[i].sh_addr;
                data = i;
                //printk("found .data at: %p\n", data);

            } else if(strcmp(".bss", name)) {
                //bss = section[i].sh_addr;
                bss = i;
                //printk("found .bss at: %p\n", data);

            }

        }

        uint64_t page, low_data_addr;
        low_data_addr =  (data < rodata)?section[data].sh_addr:section[rodata].sh_addr;

        //set up txt section
        if(PAGE_SIZE < section[txt].sh_size) {panic("ERROR: ELF txt too big\n"); halt();}
        page = insert_page(new_pml4, hdr->e_entry, USER_SETTINGS);
        memcpy((void*)page, (void*)section[txt].sh_offset, section[txt].sh_size);

        if((PAGE_SIZE + section[txt].sh_size) > low_data_addr) {
            panic("ERROR: data fits on same page as txt\n");
            halt();
        }


        struct mm_struct *mm = (struct mm_struct*)PHYS_TO_VIRT(kmalloc_pg());
        create_mm(mm,
                0, //start_stack,
                0, //mmap_base,
                0, //brk,
                0, //start_brk,
                section[bss].sh_addr,
                section[data].sh_addr,
                (data > rodata)?section[data].sh_addr:section[rodata].sh_addr,
                hdr->e_entry,
                0 //pgd
                );



    } else {
        printk("Invalid ELF header for system.\n");
    }
}


void print_elf_hdr(Elf64_Ehdr *hdr) {
    printk("hdr->e_type      %p\n", hdr->e_type);    
    printk("hdr->e_entry     %p\n", hdr->e_entry);   
    printk("hdr->e_flags     %p\n", hdr->e_flags);   
    printk("hdr->e_ident     %p\n", hdr->e_ident);   
    printk("hdr->e_phnum     %p\n", hdr->e_phnum);   
    printk("hdr->e_phoff     %p\n", hdr->e_phoff);   
    printk("hdr->e_shnum     %p\n", hdr->e_shnum);   
    printk("hdr->e_shoff     %p\n", hdr->e_shoff);   
    printk("hdr->e_ehsize    %p\n", hdr->e_ehsize);  
    printk("hdr->e_machine   %p\n", hdr->e_machine); 
    printk("hdr->e_version   %p\n", hdr->e_version); 
    printk("hdr->e_shstrndx  %p\n", hdr->e_shstrndx);
    printk("hdr->e_shentsize %p\n", hdr->e_shentsize);
    printk("hdr->e_phentsize %p\n", hdr->e_phentsize);
}

void print_elf_sec(Elf64_Shdr *section) {
    printk("section->sh_addr      %p\n", section->sh_addr);
    printk("section->sh_info      %p\n", section->sh_info);
    printk("section->sh_link      %p\n", section->sh_link);
    printk("section->sh_name      %p\n", section->sh_name);
    printk("section->sh_size      %p\n", section->sh_size);
    printk("section->sh_type      %p\n", section->sh_type);
    printk("section->sh_flags     %p\n", section->sh_flags);
    printk("section->sh_offset    %p\n", section->sh_offset);
    printk("section->sh_entsize   %p\n", section->sh_entsize);
    printk("section->sh_addralign %p\n", section->sh_addralign);
}


bool validate_header(char *data) {
    bool valid = false;
    Elf64_Ehdr *hdr = (Elf64_Ehdr*)data;
    /* Look for the magic number */
#ifdef DEBUG
    printk("Elf Header:\n");
#endif
    DEBUG_EL_MAGIC(hdr->e_ident);
    /* Print out the file format class */
    DEBUG_EL_CLASS(hdr->e_ident[EL_CLASS]);
    /* Print out the file format byte ordering */
    DEBUG_EL_DATA(hdr->e_ident[EL_DATA]);
    /* Print out the file format version */
    DEBUG_EL_VERSION(hdr->e_ident[EL_VERSION]);
    /* Print out the file format abi version */
    DEBUG_EL_ABI(hdr->e_ident[EL_ABIVERSION]);
    /* Print out the file type */
    DEBUG_EHDR_TYPE(hdr->e_type);
    /* Print out the entry address */
    DEBUG_EHDR_ENTRY_ADDR(hdr->e_entry);
    /* Print out the program header offset */
    DEBUG_EHDR_PHOFF(hdr->e_phoff);
    /* Print out the section header offset */
    DEBUG_EHDR_SHOFF(hdr->e_shoff);
    /* Print out the size of this header */
    DEBUG_EHDR_EHSIZE(hdr->e_ehsize);
    /* Print out the number of header entries */
    DEBUG_EHDR_PHSIZE(hdr->e_phentsize);
    DEBUG_EHDR_PHNUM(hdr->e_phnum);
    /* Print out the number of section entries */
    DEBUG_EHDR_SHSIZE(hdr->e_shentsize);
    DEBUG_EHDR_PHNUM(hdr->e_shnum);
    /* Print out the section header string index */
    DEBUG_EHDR_SHSTRINDEX(hdr->e_shstrndx);

    /* Check for the ELF magic number */
    if(hdr->e_ident[EL_MAG0] == 0x7f && hdr->e_ident[EL_MAG1] == 'E' && 
            hdr->e_ident[EL_MAG2] == 'L'  && hdr->e_ident[EL_MAG3] == 'F') {
        /* We only support SYSV, Little Endian, Executables */
        valid = hdr->e_ident[EL_CLASS] == ELFCLASS64 && 
            hdr->e_ident[EL_DATA]  == ELFDATA2LSB &&
            hdr->e_ident[EL_ABIVERSION] == ELFOSABI_SYSV &&
            hdr->e_type == ET_EXEC;
    }

    return valid;
}
