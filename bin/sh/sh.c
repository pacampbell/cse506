#include <stdio.h>
#include <stdlib.h>

/* Simple shell without all the fancy stuff */

#define BUFF_SIZE 50

int main(int argc, char *argv[]) {
    char str[BUFF_SIZE];

    int size = 1;
    while(size > 0) {
        printf("> ");
        size = read(0, str, BUFF_SIZE);
        printf("SIZE: %d\n", size);
        printf("GOT: %s\n", str);
    }

    return EXIT_SUCCESS;
}
