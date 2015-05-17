#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[], char *envp[]) {
    char c[BUFFER_SIZE];
    //pid_t pid;
    while(1) {
    	memset(c, 0, BUFFER_SIZE);
        printf("Enter a file, no more than %d chars: ", BUFFER_SIZE);
        printf("running: %s\n", c);
        //pid = fork();
        //if(pid == 0) {
        pid_t pid;

        switch((pid = fork())) {
        	case 0:
            execve(c, argv, envp);
            //execve("bin/args", argv, envp);
            printf("that was a bad program name, try again\n");
            return 0;
        //}
        		printf("Failed to fork.\n");
        		break;
        	default:
        		/* just continue */
        		break;
        } 
    }
}
