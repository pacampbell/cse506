#include <stdio.h>
#include <stdlib.h>

void child_func();

int main(int argc, char *argv[]) {
	pid_t pid = -1;
	printf("Hello Fork - %d\n", getpid());
	switch((pid = fork())) {
		case 0:
			printf("In the child[%d] - parent %d\n", getpid(), getppid());
			break;
		case -1:
			printf("Failed to fork\n");
			break;
		default:
			printf("Parent %d forked a child with pid %d\n", getpid(), pid);
			break;
	}
	printf("Good bye fork %d\n", getpid());
	return EXIT_SUCCESS;
}
