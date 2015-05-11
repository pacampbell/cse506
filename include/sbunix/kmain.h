#ifndef __KERNEL__
#error "Trying to include kernel code in non-kernel code."
#else
    #ifndef _KMAIN_H
    #define _KMAIN_H
		#include <sys/sbunix.h>
		#include <sys/task.h>
		#include <sys/tarfs.h>
		#include <sys/elf.h>

        void kmain(void);
        void init_services(void);
        void start_shell(void);
        void save_kernel_global(void);
    #endif
#endif
