#define __KERNEL__
#include <sys/elf.h>
#include <sbunix/string.h>
#include <sys/mm/vma.h>
#include <sys/pgtable.h>
#include <sys/screen.h>
#include <sys/task.h>
#include <sys/tarfs.h>

void print_elf_hdr(Elf64_Ehdr *hdr);
bool same_pg(uint64_t pg, uint64_t addr);
extern pml4_t* kernel_cr3;


struct mm_struct* load_elf(char *data, int len, Task *task, pml4_t *proc_pml4) {
    if (validate_header(data)) {
        pml4_t *kern_pml4 = get_cr3();
        //get the header
        Elf64_Ehdr *hdr = (Elf64_Ehdr*)data;

        //create new mm_struct
        struct mm_struct *mm = (struct mm_struct*)kmalloc_kern(PAGE_SIZE);
        memset(mm, 0, sizeof(struct mm_struct));
        if(hdr->e_shstrndx == 0x00) panic("NO STRING TABLE");

        mm->start_code = ((Elf64_Ehdr*) data)->e_entry;


        Elf64_Phdr *prgm_hdr = (Elf64_Phdr*)(data + hdr->e_phoff);
        uint64_t high_addr = 0;

        for(int i = 0;  i < hdr->e_phnum; prgm_hdr++, i++) {
            //printk("--------------LOAD-ELF-----------------\n");
            if (prgm_hdr->p_type == PT_LOAD && prgm_hdr->p_filesz > 0) {

                if (prgm_hdr->p_filesz > prgm_hdr->p_memsz) {
                    panic("Bad Elf!!!\n");
                    halt();
                }

                struct vm_area_struct *vma = (struct vm_area_struct*) kmalloc_kern(sizeof(struct vm_area_struct));
                if(kmalloc_vma(proc_pml4, prgm_hdr->p_vaddr, prgm_hdr->p_memsz, USER_SETTINGS) == NULL) {
                    panic("KMALLOC FAILED - elf.c:load_elf:34\n");
                    printk("SIZE: %d\n", prgm_hdr->p_filesz);
                }
                // printk("ELF Virtual memory address: %p\n", prgm_hdr->p_vaddr);

                set_cr3(proc_pml4);
                memset((void*)prgm_hdr->p_vaddr, 0, prgm_hdr->p_memsz);
                //printk("memcpy dest: %p src: %p size: %p\n", prgm_hdr->p_vaddr, data + prgm_hdr->p_offset, prgm_hdr->p_filesz);
                memcpy((void*)prgm_hdr->p_vaddr, data + prgm_hdr->p_offset, prgm_hdr->p_filesz);
                
                //memcpy((void*)prgm_hdr->p_vaddr, data + prgm_hdr->p_offset, prgm_hdr->p_memsz);
                set_cr3(kern_pml4);
                vma->vm_start = prgm_hdr->p_vaddr;
                vma->vm_end = (uint64_t)(prgm_hdr->p_vaddr + prgm_hdr->p_memsz);
                vma->vm_prot = prgm_hdr->p_flags;
                add_vma(mm, vma);
                if(vma->next != NULL) {
                    panic("not null\n");
                    halt();
                }

                if(vma->vm_end > high_addr) high_addr = vma->vm_end;

                if (prgm_hdr->p_vaddr == mm->start_code) {
                   // its the txt section
                   mm->end_code = (uint64_t)(prgm_hdr->p_vaddr + prgm_hdr->p_filesz);
                   mm->start_data = mm->end_code +1;
                }
            } 
        }

        high_addr += PAGE_SIZE;
        high_addr &= PG_ALIGN;

        mm->brk = high_addr;
        mm->start_brk = mm->brk;
        struct vm_area_struct *vma = (struct vm_area_struct*) kmalloc_kern(sizeof(struct vm_area_struct));
        vma->vm_start = mm->start_brk;
        vma->vm_end = mm->brk;
        vma->vm_prot = 0;
        add_vma(mm, vma);
     
        return mm;

    } else {

        return NULL;
    }

}

void load_elf_args(Task *tsk, int argc, char *argv[], char *envp[]) {
    if (tsk->mm->start_stack == 0) panic("Task not set up\n");
    
    char tmp_c;

    pml4_t *kern_cr3;
    pml4_t *task_cr3;

    kern_cr3 = get_cr3();
    task_cr3 = (pml4_t*)tsk->registers.cr3;
    set_cr3(task_cr3);

    uint64_t *new_stack = (uint64_t*)((tsk->mm->start_stack + PAGE_SIZE) & PG_ALIGN);
    if (kmalloc_vma(task_cr3, (uint64_t)new_stack, 1, USER_SETTINGS) == NULL) {
        panic("malloc failed\n");
        return;
    }
    set_cr3(kern_cr3);
    struct vm_area_struct *vma = (struct vm_area_struct *)kmalloc_kern(sizeof(struct vm_area_struct));
    vma->vm_prot = 0;
    vma->vm_start =(uint64_t) new_stack;
    vma->vm_end = vma->vm_start + PAGE_SIZE;
    add_vma(tsk->mm, vma);
    set_cr3(task_cr3);

    new_stack = (uint64_t*)tsk->mm->start_stack;

    *new_stack = argc;
    new_stack++;

    //tsk->args.argv = PHYS_TO_VIRT(kmalloc_pg());
    tsk->args.argv = (uint64_t)kmalloc_vma(task_cr3, (tsk->mm->start_stack + (4*PAGE_SIZE)) & PG_ALIGN, 1, USER_SETTINGS);
    char *tsk_argv =  (char*)tsk->args.argv;
    set_cr3(kern_cr3);
    vma = (struct vm_area_struct *)kmalloc_kern(sizeof(struct vm_area_struct));
    vma->vm_prot = 0;
    vma->vm_start =(uint64_t) tsk_argv;
    vma->vm_end = vma->vm_start + PAGE_SIZE;
    add_vma(tsk->mm, vma);
    set_cr3(task_cr3);

    for (int i = 0; i < argc; i++, new_stack++) {
        *new_stack = (uint64_t)tsk_argv;

        set_cr3(kern_cr3);
        for (int j = 0; *(argv[i]+j) != '\0'; j++, tsk_argv++) {
            //if(get_pte((pml4_t*)tsk->registers.cr3, (uint64_t)tsk_argv)) panic("VERY BAD!!!\n");
            //printk("char: %c\n", *(argv[i]+j));
            //*tsk_argv = *(argv[i] + j);
           set_cr3(kern_cr3);
           tmp_c = *(argv[i] + j);
           set_cr3(task_cr3);
           *tsk_argv = tmp_c;
           set_cr3(kern_cr3);
            //printk("%c\n", *(argv[i]+j));
            //if(i == 1 && j == 3)halt();
        }
        set_cr3(task_cr3);
        *tsk_argv = '\0';
        tsk_argv++;
    }

    *new_stack= 0;
    new_stack++;

    //tsk->args.envp = PHYS_TO_VIRT(kmalloc_pg());
    tsk->args.envp = (uint64_t)kmalloc_vma((pml4_t*)tsk->registers.cr3, (tsk->mm->start_stack + (5*PAGE_SIZE)) & PG_ALIGN, 1, USER_SETTINGS);
    char *tsk_env =  (char*)tsk->args.envp;
    set_cr3(kern_cr3);
    vma = (struct vm_area_struct *)kmalloc_kern(sizeof(struct vm_area_struct));
    vma->vm_prot = 0;
    vma->vm_start =(uint64_t) tsk_env;
    vma->vm_end = vma->vm_start + PAGE_SIZE;
    add_vma(tsk->mm, vma);
    set_cr3(task_cr3);

    //printk("adder: %p\n", new_stack);
    set_cr3(kern_cr3);
    for (int i = 0; envp[i] != NULL; i++, new_stack++) {
        set_cr3(task_cr3);
        *new_stack = (uint64_t)tsk_env;
        set_cr3(kern_cr3);

        for (int j = 0; *(envp[i]+j) != '\0'; j++, tsk_env++) {
            set_cr3(kern_cr3);
           tmp_c = *(envp[i] + j);
           set_cr3(task_cr3);
           *tsk_env = tmp_c;
           set_cr3(kern_cr3);
        }
        set_cr3(task_cr3);
        *tsk_env = '\0';
        tsk_env++;
        set_cr3(kern_cr3);
    }
    set_cr3(task_cr3);

    *new_stack= 0;
    new_stack++;
    
    set_cr3(kern_cr3);
}

bool same_pg(uint64_t pg, uint64_t addr) {
    uint64_t beg_pg = pg & PG_ALIGN;
    uint64_t end_pg = (beg_pg-1) + PAGE_SIZE;
    return beg_pg <= addr && addr <= end_pg;
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
