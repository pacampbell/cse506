#ifndef _SYS_MM_VMA_H_
#define _SYS_MM_VMA_H_

#include <sys/defs.h>
#include <sys/pgtable.h>

#define VM_READ         0x00000001      /* currently active flags */
#define VM_WRITE        0x00000002
#define VM_EXEC         0x00000004
#define VM_SHARED       0x00000008
#define VM_GROWSDOWN    0x00000100      /* general info on the segment */

struct vm_area_struct {

    uint64_t vm_start;
    uint64_t vm_end;

    struct vm_area_struct *next;
    struct vm_area_struct *prev;

    struct mm_struct *vm_mm;

    uint32_t vm_prot;

};

struct mm_struct {

    struct vm_area_struct *mmap;
    uint64_t start_stack;
    uint64_t mmap_base;
    uint64_t brk;
    uint64_t start_brk;
    uint64_t end_data;
    uint64_t start_data;
    uint64_t end_code;
    uint64_t start_code;
    uint64_t pgd; /* the page table pointer  */
};
void create_mm(struct mm_struct *mm,
               uint64_t start_stack,
               uint64_t mmap_base,
               uint64_t brk,
               uint64_t start_brk,
               uint64_t end_data,
               uint64_t start_data,
               uint64_t end_code,
               uint64_t start_code,
               uint64_t pgd
              );



#endif
