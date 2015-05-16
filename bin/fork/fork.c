#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	pid_t pid = -1;
	printf("MULTIPLY ME\n");
	switch((pid = fork())) {
		case 0:
			printf("In the child - parent %d\n", getppid());
			break;
		case -1:
			printf("Failed to fork\n");
			break;
		default:
			printf("Parent %d forked a child with pid %d\n", getpid(), pid);
			break;
	}

	return EXIT_SUCCESS;
}