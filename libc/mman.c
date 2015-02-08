#include <sys/mman.h>
#include <sys/syscall.h>

#define _SC_PAGE_SIZE 4096

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset) {
    return (void*)syscall_6(SYS_mmap, (uint64_t)addr, (uint64_t)length, (uint64_t)prot,
                    (uint64_t)flags, (uint64_t)fd, (uint64_t)offset);
}

int munmap(void *addr, size_t length) {
    return syscall_2(SYS_munmap, (uint64_t)addr, (uint64_t)length);
}
