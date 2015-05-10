#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    char c[6];
    int i = open("hi", 0);

    printf("file no: %d\n", i);

    read(i, c, 5);
    c[6] = '\0';
    printf("char: %s\n", c);

    read(i, c, 5);
    c[6] = '\0';
    printf("char: %s\n", c);


    return 0;
}
