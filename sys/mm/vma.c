#define __KERNEL__
#include <sys/mm/vma.h>

void create_mm(struct mm_struct *mm,
               uint64_t start_stack,
               uint64_t mmap_base,
               uint64_t brk,
               uint64_t start_brk,
               uint64_t end_data,
               uint64_t start_data,
               uint64_t end_code,
               uint64_t start_code
              ) {

    mm->start_stack = start_stack;
    mm->mmap_base   = mmap_base;
    mm->brk         = brk;
    mm->start_brk   = start_brk;
    mm->end_data    = end_data;
    mm->start_data  = start_data;
    mm->end_code    = end_code;
    mm->start_code  = start_code;

    struct vm_area_struct *vm_ptr = (struct vm_area_struct*)kmalloc_pg();

    //set up txt section
    mm->mmap = vm_ptr;
    vm_ptr->vm_mm = mm;
    vm_ptr->prev = NULL;
    vm_ptr->vm_start = mm->start_code;
    vm_ptr->vm_end = mm->end_code;
    vm_ptr->vm_prot = VM_READ | VM_EXEC;

    //set up data
    vm_ptr->next = (struct vm_area_struct*)kmalloc_pg();
    vm_ptr->next->prev = vm_ptr;
    vm_ptr = vm_ptr->next;
    vm_ptr->vm_start = mm->start_data;
    vm_ptr->vm_end = mm->end_data;
    vm_ptr->vm_mm = mm;
    vm_ptr->vm_prot = VM_READ | VM_WRITE;

    //set up bss
    vm_ptr->next = (struct vm_area_struct*)kmalloc_pg();
    vm_ptr->next->prev = vm_ptr;
    vm_ptr = vm_ptr->next;
    vm_ptr->vm_start = mm->end_data;
    vm_ptr->vm_end = mm->start_brk;
    vm_ptr->vm_mm = mm;
    vm_ptr->vm_prot = VM_READ | VM_WRITE;
    
    //set up heap
    vm_ptr->next = (struct vm_area_struct*)kmalloc_pg();
    vm_ptr->next->prev = vm_ptr;
    vm_ptr = vm_ptr->next;
    vm_ptr->vm_start = mm->start_brk;
    vm_ptr->vm_end = mm->brk;
    vm_ptr->vm_mm = mm;
    vm_ptr->vm_prot = VM_READ | VM_EXEC;

    //set up mem map
    vm_ptr->next = (struct vm_area_struct*)kmalloc_pg();
    vm_ptr->next->prev = vm_ptr;
    vm_ptr = vm_ptr->next;
    vm_ptr->vm_start = 0x0;
    vm_ptr->vm_end = mm->mmap_base;
    vm_ptr->vm_mm = mm;
    vm_ptr->vm_prot = VM_READ | VM_EXEC;

    //set up stack
    vm_ptr->next = (struct vm_area_struct*)kmalloc_pg();
    vm_ptr->next->prev = vm_ptr;
    vm_ptr = vm_ptr->next;
    vm_ptr->vm_start = 0x0;
    vm_ptr->vm_end = mm->start_stack;
    vm_ptr->vm_mm = mm;
    vm_ptr->vm_prot = VM_READ | VM_WRITE | VM_GROWSDOWN;

}
