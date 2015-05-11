#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    // pid_t pid = -1;
    printf("In original task.\n");
    int yield_count = 0;
    while(yield_count < 5) {
        printf("Time to yield\n");    
        yield_count++;
    }
    // switch((pid = fork())) {
    // 	case -1:
    // 		printf("Fork failed\n");
    // 		break;
    // 	case 0:
    // 		printf("In the child!!!\n");
    // 		break;
    // 	default:
    // 		printf("In parent. child: %d\n", pid);
    // 		break;
    // }
    return 1;
}

