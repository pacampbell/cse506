#include <stdio.h>
#include <stdlib.h>

void child_func();

int main(int argc, char *argv[]) {
	pid_t pid = -1;
	printf("MULTIPLY ME\n");
	switch((pid = fork())) {
		case 0:
			child_func();
			printf("In the child[%d] - parent %d\n", getpid(), getppid());
			printf("Sleeping %d for 10 seconds\n", getpid());
			sleep(10);
			break;
		case -1:
			printf("Failed to fork\n");
			break;
		default:
			printf("Parent %d forked a child with pid %d\n", getpid(), pid);
			break;
	}
	printf("Good bye world %d\n", getpid());
	return EXIT_SUCCESS;
}

void child_func() {
	pid_t pid = -1;
	switch((pid = fork())) {
		case 0:
			printf("In the child[%d] - parent %d\n", getpid(), getppid());
			printf("Sleeping %d for 10 seconds\n", getpid());
			sleep(10);
			break;
		case -1:
			printf("Failed to fork\n");
			break;
		default:
			printf("Parent %d forked a child with pid %d\n", getpid(), pid);
			break;
	}
}