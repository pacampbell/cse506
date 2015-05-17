#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[], char *envp[]) {
    char c[20];
    //pid_t pid;
    while(1) {
        printf("Enter a file, no more than 19 chars: ");
        read(0, c, 20);
        printf("running: %s\n", c);
        //pid = fork();
        //if(pid == 0) {
            execve(c, argv, envp);
            //execve("bin/args", argv, envp);
            printf("that was a bad program name, try again\n");
            return 0;
        //}
    }
}
