#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[], char *envp[]) {
    printf("execing...\n");
    execve("bin/args", argv, envp);
    printf("STILL HERE... NOT GOOD!!!\n");
}
