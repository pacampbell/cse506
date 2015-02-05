void _start();
void _init(void);
extern int main();

void _init() {

}

void _start(int argc, char **argv, char **env) {
    _init();
    // Should be exit(main())
    main(argc, argv, env);
}
