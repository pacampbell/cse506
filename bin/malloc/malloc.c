#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    char *c = (char*)malloc(0xfffffffffff);
    *c = 'p';
    *(c+1) = '\0';
    printf("malloc char: %s\n", c);
    printf("I should break now!!\n");
    *(c+(0xffffffffffe)) = '\0';
}
