#include <stdio.h>
#include <stdlib.h>

void putint(int i) {
    char buffer[256];
    char nl = '\n';
    int count = 0;
    if(i != 0) {
        // Get ascii digits in reverse
        while(i > 0) {
            buffer[count++] = (i % 10) + '0';
            i /= 10;
        }
        count--;
        // Print out the ascii digits 1 at a time
        while(count >= 0) {
            write(STDOUT_FILENO, &buffer[count--], 1);
        }
    } else {
        char z = '0';
        write(STDOUT_FILENO, &z, 1);
    }
    write(STDOUT_FILENO, &nl, 1);
}
