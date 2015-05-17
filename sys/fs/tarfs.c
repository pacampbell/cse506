#define __KERNEL__
#include <sys/tarfs.h>
#include <sbunix/string.h>

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

void ls_tars(const char* filter) {
    Header *entry = (Header*)(&_binary_tarfs_start);
    int count = 0;

    while(entry < (Header*)(&_binary_tarfs_end)) {
        uint64_t e_size = covert_base_8(entry->size);
        if (entry->name[0] != '\0' && (filter == NULL || begwith(entry->name, filter))) { 
            count++;
            printk("%s  ", entry->name);
            if(count % 5 == 0) printk("\n");
        }

        if(e_size > 0) {
            // Add the padding to the entry size
            entry = entry + 1 + (e_size / 513 + 1);
        } else {
            entry = entry + 1;
        }
    }
}

void dir_tars(const char* name, int count) {
     Header *entry = (Header*)(&_binary_tarfs_start);
     int c = 0;

    while(entry < (Header*)(&_binary_tarfs_end)) {
        uint64_t e_size = covert_base_8(entry->size);
        if (entry->name[0] != '\0' && *entry->typeflag == TAR_DIRTYPE) { 
            c++;
            printk("name: %s count: %d\n", entry->name, c);
        }

        if(e_size > 0) {
            // Add the padding to the entry size
            entry = entry + 1 + (e_size / 513 + 1);
        } else {
            entry = entry + 1;
        }
    }
   
}

tarfs_entry* traverse_tars(const char *path, tarfs_entry *t_entry) {
    tarfs_entry *found = NULL;  
    Header *entry = (Header*)(&_binary_tarfs_start);

    while(entry < (Header*)(&_binary_tarfs_end)) {
        uint64_t e_size = covert_base_8(entry->size);
        // printk("entry: %p name: %s size: %d\n", entry, entry->name, e_size);
        // See if we found the entry we are searching for.
        if(strcmp(path, entry->name)) {
            t_entry->path = path;
            t_entry->size = e_size;
            t_entry->data_base = (char*)((uint64_t)entry + BLOCK_SIZE);
            found = t_entry;
            break;
        }

        if(e_size > 0) {
            // Add the padding to the entry size
            entry = entry + 1 + (e_size / 513 + 1);
        } else {
            entry = entry + 1;
        }
    }
    // printk("name: %s\n", found->path);
    return found;
}

struct file* tarfs_to_file(const char *path) {
    tarfs_entry e;
    struct file *f = NULL;

    memset(&e, 0, sizeof(e));

    if(traverse_tars(path, &e) != NULL) {
        f = (struct file*)kmalloc_kern(sizeof(struct file));
        if (f == NULL) {
            panic("Could not make new file\n");
            return NULL;
        }

        f->start = (uint64_t)e.data_base;
        f->at = (uint64_t)e.data_base;
        f->end = (uint64_t)(e.size + e.data_base);

    } else {
        printk("Unable to find: %s in tarfs\n", path);
    }

    return f;
}

int exec_tarfs_elf(const char *path) {
    tarfs_entry e;
    memset(&e, 0, sizeof(e));
    if(traverse_tars(path, &e) != NULL) {
        /* Try to load file as elf */
        create_user_elf_task(path, e.data_base, e.size);
    } else {
        printk("Unable to find: %s in tarfs\n", path);
        return -1;
    }

    return 0;
}

int exec_tarfs_elf_args(const char *path, int argc, char *argv[], char *envp[]) {
    tarfs_entry e;
    memset(&e, 0, sizeof(e));
    if(traverse_tars(path, &e) != NULL) {
        /* Try to load file as elf */
        create_user_elf_args_task(path, e.data_base, e.size, argc, argv, envp);
    } else {
        printk("Unable to find: %s in tarfs\n", path);
        return -1;
    }

    return 0;
}

