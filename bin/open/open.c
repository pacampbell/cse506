#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    char c[3];
    int i = open("bin/hello", 0);
    printf("file no: %d\n", i);

    read(i, c, 1);
    printf("char: %c\n", *c);
    return 0;
}
