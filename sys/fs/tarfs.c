#define __KERNEL__
#include <sys/tarfs.h>

/*
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
*/

static uint64_t covert_base_8(char *str) {
	uint64_t value = 0;
	while(*str != '\0') {
		int c = *str++ - '0';
		value = value * 8 + c;
	}
	return value;
}

tarfs_entry* traverse_tars(const char *path, tarfs_entry *t_entry) {
	tarfs_entry *found = NULL;
	struct posix_header_ustar *entry = (struct posix_header_ustar *)(&_binary_tarfs_start);
	while((uint64_t)entry < (uint64_t)(&_binary_tarfs_end)) {
		uint64_t e_size = covert_base_8(entry->size);
		printk("name: %s size: %d\n", entry->name, e_size);
		// See if we found the entry we are searching for.
		if(strcmp(path, entry->name)) {
			t_entry->path = path;
			t_entry->size = e_size;
			t_entry->data_base = (char*)((char*)entry + sizeof(struct posix_header_ustar));
			found = t_entry;
			break;
		}
		// If not get the next tars entry
		entry = (struct posix_header_ustar *)((char*)entry + sizeof(struct posix_header_ustar) + e_size);
		if(e_size > 0) {
			// Compute the padding size
			int padding = BLOCK_SIZE - (e_size % BLOCK_SIZE);
			// Add the padding to the entry size
			entry = (struct posix_header_ustar *)((char*)entry + padding);
		}
	}
	return found;
}

void exec_tarfs_elf(const char *path) {
	tarfs_entry e;
	memset(&e, 0, sizeof(e));
    if(traverse_tars(path, &e) != NULL) {
        /* Try to load file as elf */
        create_user_elf_task(path, e.data_base, e.size);
    } else {
        printk("Unable to find: %s in tarfs\n", path);
    }
}