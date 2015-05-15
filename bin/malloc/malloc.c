#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    char *c = (char*)malloc(5);
    *c = 'p';
    *(c+1) = '\0';

    printf("malloc char: %s\n", c);
    //c = (char*) malloc(999999);
}
