#define __KERNEL__
#include <sys/tarfs.h>
#include <sys/sbunix.h>

static char* type(char t) {
	switch(t) {
		case TAR_REGTYPE:
		case TAR_AREGTYPE:
			return "-";
		case TAR_LNKTYPE:
			return "Link";
		case TAR_SYMTYPE:
			return "Reserved";
		case TAR_CHRTYPE:
			return "Character Special";
		case TAR_BLKTYPE:
			return "Block Special";
		case TAR_DIRTYPE:
			return "d";
		case TAR_FIFOTYPE:
			return "Fifo";
		case TAR_CONTTYPE:
			return "Reserved(%c)";
		default:
			printk("Unknown tarfs type: %d\n", t);
			return "";
	}
}

static uint64_t covert_base_8(char *str) {
	uint64_t value = 0;
	while(*str != '\0') {
		int c = *str++ - '0';
		value = value * 8 + c;
	}
	return value;
}

void traverse_tars(void) {
	uint64_t size = &_binary_tarfs_end - &_binary_tarfs_start;
	printk("Tarfs Size: %d bytes\n", size);
	printk("start: %p\n", &_binary_tarfs_start);
	printk("end: %p\n", &_binary_tarfs_end);

	struct posix_header_ustar *entry = (struct posix_header_ustar *)&_binary_tarfs_start;
	while((uint64_t)entry < (uint64_t)(&_binary_tarfs_end)) {
		uint64_t e_size = covert_base_8(entry->size);
		printk("%s %d %s\n", type(entry->typeflag[0]), e_size, entry->name);
		entry = (struct posix_header_ustar *)((char*)entry + sizeof(struct posix_header_ustar) + e_size);
		if(e_size > 0) {
			// Compute the padding size
			int padding = BLOCK_SIZE - (e_size % BLOCK_SIZE);
			// Add the padding to the entry size
			entry = (struct posix_header_ustar *)((char*)entry + padding);
		}
	}
}