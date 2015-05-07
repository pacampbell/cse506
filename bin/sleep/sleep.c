#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
	if(argc > 1) {
		int duration = convert(argv[1], 10);
		if(duration > 0) {
			sleep((unsigned int)duration);
		}
	} else {
		printf("usage: %s duration\n", argv[0]);
	}
	return EXIT_SUCCESS;
}