#ifndef _SCREEN_H
#define _SCREEN_H

#define VIDEO_MEM_START 0xb8000
#define VIDEO_MEM (volatile char*)(VIDEO_MEM_START)
#define TERMINAL_COLUMNS 80
#define TERMINAL_ROWS 25
#define VIDEO_MEM_END_OFFSET (TERMINAL_COLUMNS * TERMINAL_ROWS)
#define VIDEO_MEM_END ((VIDEO_MEM_START) + (VIDEO_MEM_END_OFFSET))

/* Clears the terminal */
void cls(void);

/**
 * Puts a character at the current cursor with the default color.
 * @param c Char to write to the terminal.
 */
void putk(char c);

/**
 * Puts a character at the current cursor with a color values.
 * @param color 1-byte containing the background and forground color.
 * @param c Char to write to the terminal.
 */
void putck(char color, char c);

/**
 * Seeks to a position in video memory.
 * @param offset Index to seek to based on the current position of
 * of the cursor.
 * @return Returns the video address based on the provided offset.
 */
volatile char *video_seek(int offset);

#endif
