#ifndef _TARFS_H
#define _TARFS_H

#include <sys/sbunix.h>
#include <sbunix/string.h>
#include <sys/defs.h>
#include <sys/task.h>

extern char _binary_tarfs_start;
extern char _binary_tarfs_end;

/* http://www.subspacefield.org/~vax/tar_format.html */
#define BLOCK_SIZE 512 

#define TAR_REGTYPE '0'			/* Regular File */
#define TAR_AREGTYPE '\0'		/* Regular File */
#define TAR_LNKTYPE	'1'			/* link */
#define TAR_SYMTYPE '2'			/* reserved */
#define TAR_CHRTYPE '3'			/* character special */
#define TAR_BLKTYPE '4'			/* block special */
#define TAR_DIRTYPE '5'			/* directory - Size has no meaning */
#define TAR_FIFOTYPE '6'		/* fifo */
#define TAR_CONTTYPE '7'		/* reserved */

struct posix_header_ustar {
	char name[100];
	char mode[8];
	char uid[8];
	char gid[8];
	char size[12];
	char mtime[12];
	char checksum[8];
	char typeflag[1];
	char linkname[100];
	char magic[6];
	char version[2];
	char uname[32];
	char gname[32];
	char devmajor[8];
	char devminor[8];
	char prefix[155];
	char pad[12];
};

struct tarfs_entry {
	const char *path;		/* Path of the file in tarfs */
	char *data_base;		/* Base address of file */
	uint64_t size;			/* How large the file is in bytes */
};
typedef struct tarfs_entry tarfs_entry;

tarfs_entry* traverse_tars(const char *path, tarfs_entry *t_entry);
void exec_tarfs_elf(const char *path);

#endif
