#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    char *c = (char*)malloc(0xfffffffffff);
    *c = 'p';
    *(c+1) = '\0';
    *(c+(0xffffffffffe)) = '\0';

    printf("malloc char: %s\n", c);
    //c = (char*) malloc(999999);
}
