#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[], char *envp[]) {
    char c[BUFFER_SIZE];
    //pid_t pid;
    while(1) {
        //pid_t pid;
    	memset(c, 0, BUFFER_SIZE);
        printf("Enter a file, no more than %d chars: ", BUFFER_SIZE);
        int i = read(0, c, BUFFER_SIZE);
        if(i < 1) continue;
        printf("running: %s\n", c);
        //pid = fork();
        //if(pid == 0) {

            //execve(c, argv, envp);
            execve(c, argv, envp);
            printf("that was a bad program name, try again\n");
            return 0;
        //}
    }
}
