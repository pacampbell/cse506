#define __KERNEL__

#include <sys/sbunix.h>
#include <sys/gdt.h>
#include <sys/idt.h>
#include <sys/tarfs.h>
#include <sys/keyboard.h>
#include <sys/task.h>
#include <sbunix/kernel.h>
#include <sys/pgtable.h>
#include <sbunix/kmain.h>
#include <sys/syscall_k.h>
#include <sys/exceptions.h>

void *kern_free;
void *kern_base;

void start(uint32_t* modulep, void* physbase, void* physfree) {
    struct smap_t {
        uint64_t base, length;
        uint32_t type;
    }__attribute__((packed)) *smap;
    // Save pointers for kernel
    kern_free = physfree;
    kern_base = physbase;
    // Initialize the page free list
    init_free_pg_list(physfree);
    physfree = kern_free;
    while(modulep[0] != 0x9001) modulep += modulep[1]+2;
    for(smap = (struct smap_t*)(modulep+2); smap < (struct smap_t*)((char*)modulep+modulep[1]+2*4); ++smap) {
        if (smap->type == 1 /* memory */ && smap->length != 0) {
            printk("Available Physical Memory [%x-%x]\n", smap->base, smap->base + smap->length);
            set_kern_pg_used(smap->base,smap->base + smap->length);
        }
    }
    printk("tarfs in [%p:%p]\n", &_binary_tarfs_start, &_binary_tarfs_end);
    // register all fault handlers
    initialize_fault_handlers();
    initializePaging((uint64_t)physbase, (uint64_t)physfree);
    // Set up the syscall table
    init_syscall();
    // Create the kmain task
    Task *kmain_task = create_kernel_task("KMAIN", kmain);
    // dump_task(kmain_task);
    // Setup timer and keyboard here
    init_timer(50);
    init_keyboard();
    __asm("sti");
    // Start kmain
    set_task(kmain_task);

    // test pg fault
    // uint64_t *ptr = (uint64_t*)0xA000000000000000;
    // uint64_t do_page_fault = *ptr;
    // printk("PAGEFAULT: %c!!!!!!!!\n", (char) do_page_fault);
}

#define INITIAL_STACK_SIZE 4096
char stack[INITIAL_STACK_SIZE];
uint32_t* loader_stack;
extern char kernmem, physbase;
struct tss_t tss;

void boot(void) {
    // note: function changes rsp, local stack variables can't be practically used
    // register char *s, *v;
    __asm__(
            "movq %%rsp, %0;"
            "movq %1, %%rsp;"
            :"=g"(loader_stack)
            :"r"(&stack[INITIAL_STACK_SIZE])
           );
    // Initialize the descript tables and tss
    reload_gdt();
    setup_tss();
    init_idt();
    start(
            (uint32_t*)((char*)(uint64_t)loader_stack[3] + (uint64_t)&kernmem - (uint64_t)&physbase),
            &physbase,
            (void*)(uint64_t)loader_stack[4]
         );
    while(1);
}
