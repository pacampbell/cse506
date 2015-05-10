#define __KERNEL__
#include <sys/fs/file.h>

void free_file_list(struct file *f[], size_t size) {
    for (int i = 0; i < size; i++) {
        if(f[i] != NULL) {
            kfree_pg(f[i]);
            f[i] = NULL;
        }
    }
}
