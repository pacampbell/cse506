#include <sys/screen.h>

static int cursor_x = 0, cursor_y = 0;

void cls(void) {
    for(int i = 0; i < VIDEO_MEM_END_OFFSET; i++) {
        *(VIDEO_MEM + i) = 0;
    }
    // Reset cursor positions
    cursor_x = cursor_y = 0;
}

void putk(char c) {
    putck(0, c);
}

void putck(char color, char c) {
    if(c != '\n') {
        // Write the character
        *(VIDEO_MEM + (cursor_y * TERMINAL_COLUMNS) + cursor_x) = c;
        // Write the color information
        *(VIDEO_MEM + (cursor_y * TERMINAL_COLUMNS) + cursor_x + 1) = color;
        // Increment the current cursor by 1 whole position (2-bytes)
        cursor_x += 2;
        // Check to see if we should increment by column
    }
    if(c == '\n' || cursor_x == 80) {
        cursor_x = 0;
        cursor_y += 1;
    }
}


volatile char *video_seek(int offset) {
    return 0;
}
