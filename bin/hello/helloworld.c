#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    pid_t c_pid = -1;
    printf("Hello, World! from user space.\n");

    switch((c_pid = fork())) {
    	case -1:
    		printf("Fork failed\n");
    		break;
    	case 0:
    		printf("In the child!!!\n");
    		break;
    	default:
    		printf("In parent: %d child pid: %d\n", getpid(), c_pid);
    		break;
    }
    return 1;
}

