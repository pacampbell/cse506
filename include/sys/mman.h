#ifndef _MMAN_H
#define _MMAN_H
#include <syscall.h>
#include <sys/defs.h>

#define PROT_NONE 0x00
#define PROT_READ 0x01
#define PROT_WRITE 0x02
#define PROT_EXEC 0x04

#define	MAP_SHARED 0x001
#define	MAP_PRIVATE	0x002
#define MAP_FIXED 0x010

#define MAP_ANONYMOUS 0x20
#define _SC_PAGE_SIZE 4096

#define MAP_FAILED ((void*)-1)

typedef uint64_t off_t;
typedef uint64_t size_t;

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int munmap(void *addr, size_t length);

#endif
