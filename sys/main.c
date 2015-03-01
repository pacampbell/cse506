#define __KERNEL__

#include <sys/sbunix.h>
#include <sys/gdt.h>
#include <sys/idt.h>
#include <sys/tarfs.h>

void start(uint32_t* modulep, void* physbase, void* physfree)
{
	struct smap_t {
		uint64_t base, length;
		uint32_t type;
	}__attribute__((packed)) *smap;
	while(modulep[0] != 0x9001) modulep += modulep[1]+2;
	for(smap = (struct smap_t*)(modulep+2); smap < (struct smap_t*)((char*)modulep+modulep[1]+2*4); ++smap) {
		if (smap->type == 1 /* memory */ && smap->length != 0) {
			printk("Available Physical Memory [%x-%x]\n", smap->base, smap->base + smap->length);
		}
	}
	printk("tarfs in [%p:%p]\n", &_binary_tarfs_start, &_binary_tarfs_end);

	__asm__ __volatile__("int $0x0");
	__asm__ __volatile__("int $0x1");

	// kernel starts here
	// cls();
	char *test = "\nHello, World! Do I work? I need to test out a really really long string. like super duper long so it takes up a lot of characters\nDoes this newline actually work?\rOverwrite the line we are currently at.";
	while(*test) {
		putk(*test++);
		for(int i = 0; i < 10000000; i++) ;
	}
	/*
	cls();
	test = "Writing another string.";
	while(*test) {
		putk(*test++);
		for(int i = 0; i < 10000000; i++) ;
	}
	*/
}

#define INITIAL_STACK_SIZE 4096
char stack[INITIAL_STACK_SIZE];
uint32_t* loader_stack;
extern char kernmem, physbase;
struct tss_t tss;

void boot(void)
{
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
	init_idt();
	setup_tss();
	start(
		(uint32_t*)((char*)(uint64_t)loader_stack[3] + (uint64_t)&kernmem - (uint64_t)&physbase),
		&physbase,
		(void*)(uint64_t)loader_stack[4]
	);
	/*
	s = "!!!!! start() returned !!!!!";
	for(v = (char*)0xb8000; *s; ++s, v += 2) *v = *s;
	*/
	while(1);
}
