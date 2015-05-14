#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    // pid_t pid = -1;
    printf("Hello, World! from user space.\n");
    sleep(1000);
    //  int yield_count = 0;
    // while(yield_count < 6) {
    //     // printf("Time to yield\n");    
    //     yield();
    //     yield_count++;
    // }
    // sleep(1000);
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
    // printf("Good bye world!\n");
    return 1;
}

