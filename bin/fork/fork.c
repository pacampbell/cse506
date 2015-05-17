#include <stdio.h>
#include <stdlib.h>

void child_func();

int main(int argc, char *argv[]) {
	pid_t pid = -1;
	printf("MULTIPLY ME\n");
	switch((pid = fork())) {
		case 0:
			printf("=====CCCCCC======\n");
			printf("In the child[%d] - parent %d\n", getpid(), getppid());
			child_func();
			break;
		case -1:
			printf("Failed to fork\n");
			break;
		default:
			printf("=====PPPPP======\n");
			printf("Parent %d forked a child with pid %d\n", getpid(), pid);
			break;
	}
	printf("Good bye world\n");
	return EXIT_SUCCESS;
}

void child_func() {
	pid_t pid = -1;
	switch((pid = fork())) {
		case 0:
			printf("In the child[%d] - parent %d\n", getpid(), getppid());
			// child_func();
			break;
		case -1:
			printf("Failed to fork\n");
			break;
		default:
			printf("Parent %d forked a child with pid %d\n", getpid(), pid);
			break;
	}
}