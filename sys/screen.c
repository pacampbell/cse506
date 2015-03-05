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

void setxy(int x, int y) {
    cursor_x = x;
    cursor_y = y;
}

void putck(char color, char c) {
    // If we have run out of rows scroll the screen
    if(cursor_y >= TERMINAL_ROWS) {
        scroll();
        cursor_y -= 1;
    }
    // Just dump the text if the string contains ascii
    if(c != '\n' && c != '\r' && c != '\b') {
        // Write the character
        *(VIDEO_MEM + (cursor_y * (TERMINAL_COLUMNS * 2)) + cursor_x) = c;
        // Write the color information
        *(VIDEO_MEM + (cursor_y * (TERMINAL_COLUMNS * 2)) + cursor_x + 1) = color;
        // Increment the current cursor by 1 whole position (2-bytes)
        cursor_x += 2;
    }
    // Begin checking for special situations
    if(c == '\n' || cursor_x == (TERMINAL_COLUMNS * 2)) {
        // Increment to the next row and reset the column
        cursor_x = 0;
        cursor_y += 1;
    } else if(c == '\r') {
        // Set the cursor back to the start of the current row
        cursor_x = 0;
    } else if(c == '\b') {
        // Go back by 1 character in the current row
        cursor_x = cursor_x - 2 < 0 ? 0 : cursor_x - 2;
    }
}


volatile char *video_seek(int offset) {
    volatile char *address = VIDEO_MEM + (cursor_x + (cursor_y * 2) + (offset * 2));
    if(address < VIDEO_MEM) {
        address = VIDEO_MEM;
    } else if(address > VIDEO_MEM_END) {
        address = VIDEO_MEM_END;
    }
    return address;
}

void scroll(void) {
    int row_offset = TERMINAL_COLUMNS * 2;
    // Copy everything up
    for(int i = row_offset; i < VIDEO_MEM_END_OFFSET; i++) {
        *(VIDEO_MEM + i - row_offset) = *(VIDEO_MEM + i);
    }
    // Clear the last row
    for(int i = VIDEO_MEM_END_OFFSET - row_offset; i < VIDEO_MEM_END_OFFSET; i++) {
        *(VIDEO_MEM + i) = 0;
    }
}
