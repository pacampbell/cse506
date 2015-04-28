#define __KERNEL__
#include <sys/elf.h>


void load_elf(char *data, uint64_t length) {
	if(validate_header(data)) {
		printk("Valid ELF header for system.");
	} else {
		printk("Invalid ELF header for system.");
	}
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