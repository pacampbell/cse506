void __init();
void __start();
extern int main();

void __init() {

    __start();

}

void __start() {
    main();
}
