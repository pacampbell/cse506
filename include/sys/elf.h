#ifndef __KERNEL__
#error "Trying to include kernel code in non-kernel code."
#else
	#ifndef __ELF_H
	#define __ELF_H
	#include <sys/defs.h>
	#include <sys/sbunix.h>
	/* TODO: Does not include dynamic linking tables */

	/* Elf-64 typedefs defined from ELF-64 Object File Format, Version 1.5 
	   Page 2 
	 */
	typedef uint64_t Elf64_Addr;
	typedef uint64_t Elf64_Off;
	typedef uint16_t Elf64_Half;
	typedef uint32_t Elf64_Word;
	typedef int32_t Elf64_Sword;
	typedef uint64_t Elf64_Xword;
	typedef int64_t Elf64_Sxword;


	/* e_ident constants - Page 3, 4 */
	#define EL_MAG0 0 				
	#define EL_MAG1 1
	#define EL_MAG2 2
	#define EL_MAG3 3
	#define EL_CLASS 4				
	#define EL_DATA  5				
	#define EL_VERSION 6			
	#define EL_OSABI 7				
	#define EL_ABIVERSION 8			
	#define EL_PAD 9				
	#define EL_NIDENT 16

	/* e_type constants */
	#define ET_NONE 0				/*      No file type     */
	#define ET_REL 1				/* Relocatable file type */
	#define ET_EXEC 2				/* Executeable file type */
	#define ET_DYN 3				/* Shared Objct file type */
	#define ET_CORE 4				/* Core File */
	#define ET_LOOS 0xfe00			/* Environment-specific use */
	#define ET_HIOS 0xfeff
	#define ET_LOPROC 0xff00		/* Processor-specfific use */
	#define ET_HIPROC 0xffff

	/* Values to check for */
	#define ELFCLASS32 1
	#define ELFCLASS64 2

	#define ELFDATA2LSB 1
	#define ELFDATA2MSB 2

	#define ELFOSABI_SYSV 0
	#define ELFOSABI_HPUX 1
	#define ELFOSABI_STANDALONE 255

	


	/* Elf Header Struct */ 
	typedef struct {
		unsigned char e_ident[16];  /* ELF Identification */
		Elf64_Half e_type;			/* Object file type */
		Elf64_Half e_machine;		/* Machine Type */
		Elf64_Word e_version;		/* Object file version */
		Elf64_Addr e_entry;			/* Entry point address */
		Elf64_Off e_phoff;			/* Program header offset */
		Elf64_Off e_shoff;			/* Section header offset */
		Elf64_Word e_flags;			/* Processor-specific flags */
		Elf64_Half e_ehsize;		/* ELF header size */
		Elf64_Half e_phentsize;		/* Size of program header entry */
		Elf64_Half e_phnum;			/* Nuber of program header entries */
		Elf64_Half e_shentsize;		/* Size of section header entry */
		Elf64_Half e_shnum;			/* Number of section header entries */
		Elf64_Half e_shstrndx;		/* Section name string table index */
	} Elf64_Ehdr;

	/* sh_type constants */
	#define SHT_NULL 0
	#define SHT_PROGBITS 1
	#define SHT_SYMTAB 2
	#define SHT_STRTAB 3
	#define SHT_RELA 4
	#define SHT_HASH 5
	#define SHT_DYNAMIC 6
	#define SHT_NOTE 7
	#define SHT_NOBITS8
	#define SHT_REL 9
	#define SHT_SHLIB 10
	#define SHT_DYNSYM 11
	#define SHT_LOOS 0x60000000
	#define SHT_HIOS 0x6fffffff
	#define SHT_LOPROC 0x70000000
	#define SHT_HIPROC 0x7fffffff

	/* sh_flags constants */
	#define SHF_WRITE 0x1
	#define SHF_ALLOC 0x2
	#define SHF_EXECINSTR 0x4
	#define SHF_MASKOS 0x0f000000
	#define SHF_MASKPROC 0xf0000000

	/* Elf64 Section Header Struct */
	typedef struct {
		Elf64_Word sh_name;			/* Section name */
		Elf64_Word sh_type;			/* Section type */
		Elf64_Xword sh_flags;		/* Section Attributes */
		Elf64_Addr sh_addr;			/* Virtual address in memory */
		Elf64_Off sh_offset;		/* Offset in file */
		Elf64_Xword sh_size;		/* Size of section */
		Elf64_Word sh_link;			/* Link to other section */
		Elf64_Word sh_info;			/* Misc information */
		Elf64_Xword sh_addralign;	/* Address alignment boundry */
		Elf64_Xword sh_entsize;		/* Size of entries, if section has table */
	} Elf64_Shdr;

	/* symbol table binding constants */
	#define STB_LOCAL 0
	#define STB_GLOBAL 1
	#define STB_WEAK 2
	#define STB_LOOS 10
	#define STB_HIOS 12
	#define STB_LOPROC 13
	#define STB_HIPROC 15

	/* symbol table type constants */
	#define STT_NOTYPE 0
	#define STT_OBJECT 1
	#define STT_FUNC 2
	#define STT_SECTION 3
	#define STT_FILE 4
	#define STT_LOOS 10
	#define STT_HIOS 12
	#define STT_LOPROC 13
	#define STT_HIPROC 15

	/* Elf64 Symbol table struct */
	typedef struct {
		Elf64_Word st_name;			/* Symbol Name */
		unsigned char st_info;		/* Type and Binding attributes */
		unsigned char st_other;		/* Reserved */
		Elf64_Half st_shndx;		/* Section table index */
		Elf64_Addr st_value;		/* Symbol value */
		Elf64_Xword st_size;		/* Size of object (e.g., common)*/
	} Elf64_Sym;

	/* shorthand relocation struct */
	typedef struct {
		Elf64_Addr r_offset;		/* Address of reference */
		Elf64_Xword r_info;			/* Symbol index and type of relocation */
	} Elf64_Rel;

	/* full relocation struct */
	typedef struct {
		Elf64_Addr r_offset;		/* Address of reference */
		Elf64_Xword r_info;			/* Symbol index and type of relocation */
		Elf64_Sxword r_addend;		/* Constant part of expression */
	} Elf64_Rela;


	/* program header type constants */
	#define PT_NULL 0
	#define PT_LOAD 1
	#define PT_DYNAMIC 2
	#define PT_INTERP 3
	#define PT_NOTE 4
	#define PT_SHLIB 5
	#define PT_PHDR 6
	#define PT_LOOS 0x60000000
	#define PT_HIOS 0x6fffffff
	#define PT_LOPROC 0x70000000
	#define PT_HIPROC 0x7fffffff

	/* program header flags constants */
	#define PF_X 0x1
	#define PF_W 0x2
	#define PF_R 0x4
	#define PF_MASKOS 0x00ff0000
	#define PF_MASKPROC 0xff000000

	/* program header struct */
	typedef struct {
		Elf64_Word p_type;			/* Type of segment */
		Elf64_Word p_flags;			/* Segment attributes */
		Elf64_Off p_offset;			/* Offset in file */
		Elf64_Addr p_vaddr;			/* Virtual address in memory */
		Elf64_Addr p_paddr;			/* Reserved */
		Elf64_Xword p_filesz;		/* Size of segement in file */
		Elf64_Xword p_memsz;		/* Size of segment in memory */
		Elf64_Xword p_align;		/* Alignment of segement */
	} Elf64_Phdr;

	/* Helper functions */
        struct mm_struct* load_elf(char *data, uint64_t length, struct pml4_t *new_pml4);
	bool validate_header(char *data);
	

	/* Debug macros */
	#ifdef DEBUG
		#define DEBUG_EL_MAGIC(e_ident) do { \
			printk("\tMagic: %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x\n", \
				/* 0x7f 'E' 'L' 'F' */ \
				e_ident[EL_MAG0], e_ident[EL_MAG1], \
				e_ident[EL_MAG2], e_ident[EL_MAG3], \
				/* 32 or 64 bit */ \
				e_ident[EL_CLASS], \
				/* Little Endian or Big Endian */ \
				e_ident[EL_DATA], \
				/* File Version */ \
				e_ident[EL_VERSION], \
				/* OS-Abi Identification */ \
				e_ident[EL_ABIVERSION], \
				/* Padding Bytes begin */ \
				e_ident[9],  e_ident[10], e_ident[11], e_ident[12],\
				e_ident[13], e_ident[14], e_ident[15] \
			); \
		} while(0)

		#define DEBUG_EL_CLASS(class) do { \
	        printk("\tClass: %s\n", class == 1 ? "ELF32" : class == 2 ? "ELF64" : "UNKNOWN"); \
	    } while(0)

	    #define DEBUG_EL_DATA(data) do { \
	        printk("\tData: %s\n", data == 1 ? "2's complement, little endian" : data == 2 ? "2's complement, big endian" : "UNKNOWN"); \
	    } while(0)

	    #define DEBUG_EL_VERSION(version) do { \
	        printk("\tVersion: %s\n", version == 1 ? "1 (current)" : "UNKNOWN Extension"); \
	    } while(0)

	    #define DEBUG_EL_ABI(abi) do { \
	        printk("\tOS/ABI: %s\n", abi == 0 ? "System V ABI" : abi == 1 ? "HP-UX Operating System" : abi == 255 ? "Standalone" : "UNKNOWN"); \
	    	printk("\tABI Version: %d\n", abi); \
	    } while(0)

	    #define DEBUG_EHDR_TYPE(type) do { \
	    	printk("\tType: %s\n", type == ET_NONE ? "No file type" : \
	    		                   type == ET_REL ? "Relocatable object file" : \
	    		                   type == ET_EXEC ? "Executable file" : \
	    		                   type == ET_DYN ? "Shared object file" : \
	    		                   type == ET_CORE ? "Core file" : \
	    		                   "Environment-specific or Processor-specific \
	    		                   value" ); \
		} while(0)
	
		#define DEBUG_EHDR_ENTRY_ADDR(address) printk("\tEntry point address: %p\n", address)
		#define DEBUG_EHDR_PHOFF(offset) printk("\tStart of program headers: %d (bytes into file)\n", offset)
	    #define DEBUG_EHDR_SHOFF(offset) printk("\tStart of section headers: %d (bytes into file)\n", offset)
	    #define DEBUG_EHDR_EHSIZE(size)	printk("\tSize of this header: %d (bytes)\n", size)
	    #define DEBUG_EHDR_PHSIZE(size) printk("\tSize of program headers: %d (bytes)\n", size)
	    #define DEBUG_EHDR_SHSIZE(size) printk("\tSize of section headers: %d (bytes)\n", size)
	    #define DEBUG_EHDR_SHSTRINDEX(index) printk("\tSection header string table index: %d\n", index)
		#define DEBUG_EHDR_PHNUM(num) printk("\tNumber of program headers: %d\n", num)
	    #define DEBUG_EHDR_SHNUM(num) printk("\tNumber of section headers: %d\n", num)
	#else
		/* e_ident[16] macros */
	    #define DEBUG_EL_MAGIC(e_ident)
	    #define DEBUG_EL_CLASS(class)
	    #define DEBUG_EL_DATA(data)
	    #define DEBUG_EL_VERSION(data)
	    #define DEBUG_EL_ABI(abi)
	    /* Elf64_Ehdr macros */
	    #define DEBUG_EHDR_TYPE(type)
	    #define DEBUG_EHDR_ENTRY_ADDR(address)
	    #define DEBUG_EHDR_PHOFF(offset)
	    #define DEBUG_EHDR_SHOFF(offset)
	    #define DEBUG_EHDR_EHSIZE(size)
	    #define DEBUG_EHDR_PHSIZE(size)
	    #define DEBUG_EHDR_SHSIZE(size)
	    #define DEBUG_EHDR_SHSTRINDEX(index)
	    #define DEBUG_EHDR_PHNUM(num)
	    #define DEBUG_EHDR_SHNUM(num)
	#endif	/* DEBUG endif */
	#endif /* __ELF_H endif */
#endif
