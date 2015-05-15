#ifndef _DEFS_H
#define _DEFS_H

#ifndef NULL
    #define NULL ((void*)0)
#endif  /* NULL */

typedef unsigned long  uint64_t;
typedef          long   int64_t;
typedef unsigned int   uint32_t;
typedef          int    int32_t;
typedef unsigned short uint16_t;
typedef          short  int16_t;
typedef unsigned char   uint8_t;
// typedef          char    int8_t;

typedef uint64_t size_t;
typedef int64_t ssize_t;
typedef long off_t;

/* boolean typedefs */
typedef enum _bool {
    true = 1,
    false = 0
} bool;

#endif /* _DEFS_H */
