#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[], char *envp[]) {
    char c[BUFFER_SIZE];
    while(1) {
    	memset(c, 0, BUFFER_SIZE);
        printf("Enter a file, no more than %d chars: ", BUFFER_SIZE);
        read(0, c, BUFFER_SIZE - 1);
        printf("execing: %s\n", c);
        
        pid_t pid;

        switch((pid = fork())) {
        	case 0:
        		execve(c, argv, envp);
        		/* should never get here */
        		printf("that was a bad program name, try again\n");
        		break;
        	case -1:
        		printf("Failed to fork.\n");
        		break;
        	default:
        		/* just continue */
        		break;
        } 
    }
}
