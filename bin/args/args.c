#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[], char *envp[]) {
    printf("======= args test ==========\n");
    printf("argc: %d\n", argc);

    for (int i = 0; i < argc; ++i) {
        printf("argv: %s\n", argv[i]);
    }


    printf("adder in stack: %x\n", envp);
    printf("adder in string: %x\n", *envp);
    for(int i = 0; envp[i] != NULL; i++) {
        printf("envp: %s\n", envp[i]);
    }    
    printf("============================\n");
    return 0;
}
