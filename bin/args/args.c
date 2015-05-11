#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[], char *envp[]) {
    printf("======= args test ==========\n");
    printf("argc: %d\n", argc);

    for (int i = 0; i < argc; ++i) {
        printf("argv: %s\n", argv[i]);
    }       
    printf("============================\n");
    return 0;
}
