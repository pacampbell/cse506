#include <sys/sbunix.h>

void printk(const char *fmt, ...) {
    /* Currently does nothing */
    volatile char *video_memory = (volatile char*)0xb8000;
    while(*fmt != '\0') {
        *video_memory = *fmt++;
        // 2 bytes for each output
        // |    Ascii Code   | Color Information |
        // |    1-byte       |   1-byte          |
        video_memory += 2;
    }
}
