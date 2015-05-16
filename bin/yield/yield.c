#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
	int yield_count = 0;
	while(yield_count < 5) {
		printf("%d is yielding - count %d\n", getpid(), yield_count + 1);
		yield_count++;
		yield();
	}

	return EXIT_SUCCESS;
}