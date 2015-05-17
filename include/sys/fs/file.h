#ifndef _SYS_FS_FILE_H_
#define _SYS_FS_FILE_H_

#include <sys/defs.h>
#include <sys/pgtable.h>

#define MAX_FD 50
#define MAX_PATH 25

#define SEEK_SET        0       /* seek relative to beginning of file */
#define SEEK_CUR        1       /* seek relative to current file position */
#define SEEK_END        2       /* seek relative to end of file */
#define SEEK_DATA       3       /* seek to the next data */
#define SEEK_HOLE       4       /* seek to the next hole */
#define SEEK_MAX        SEEK_HOLE

struct file {
    uint64_t start;
    uint64_t end;
    uint64_t at;
    
    char path[MAX_PATH];
};

void free_file_list(struct file *f[], size_t size);
int close_file(struct file *f[], int fd);

#endif
