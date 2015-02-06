#include <stdlib.h>
void _start(int argc, char **argv, char **envp);
void _init(void);
extern int main();

void _init() {

}

void _start(int argc, char **argv, char **envp) {
    int result = 0;
    _init();
    result = main(argc, argv, envp);
    exit(result);
/*
#include <stdlib.h>

void _start(void) {
	int argc = 1;
	char* argv[0];
	char* envp[0];
	int res;
	res = main(argc, argv, envp);
	exit(res);
*/
}
