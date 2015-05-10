#ifndef _SYS_FS_FILE_H_
#define _SYS_FS_FILE_H_

#include <sys/defs.h>
#include <sys/pgtable.h>

#define MAX_FD 50
#define MAX_PATH 25

struct file {
    uint64_t start;
    uint64_t end;
    uint64_t at;
    
    char path[MAX_PATH];
};

void free_file_list(struct file *f[], size_t size);

#endif
