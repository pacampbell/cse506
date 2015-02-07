void _start(void);
extern int main();
extern void exit();

/**
    0
    ....
    envp[0]          <- rsp + argc + 1
    0                <- rsp + argc
    ....
    argv[0]          <- rsp + 8
    argc             <- rsp
============
Register arguments calling conventions

%rdi
%rsi
%rdx
%r10
%r8
%r9
 */
void _start(void) {
    __asm__ __volatile__ (
        ".intel_syntax;"
        "mov %%rbp, %%rsp;"                     // ; rbp = rsp
        "mov %%rdi, [%%rsp];"                   // ; This should be argc

        "mov %%rax, %%rsp;"                     // ; copy the stack pointer
        "add %%rax, 0x8;"                       // ; add 8 to the offset
        "mov %%rsi, %%rax;"                     // ; This should be argv[0]

        "mov %%rdx, %%rax;"                     // ; store argv[0]
        "mov %%rax, %%rdi;"                     // ; get argc
        "imul %%rax, 0x8;"                      // ; argc * 8
        "add %%rax, 0x8;"                       // ; (argc * 8) + 8
        "add %%rdx, %%rax;"                     // ; This should be envp[0]

        "call main;"                            // ; call main
        "mov %%rdi, %%rax;"                     // ; Set exits first param to be the return value of main
        "call exit;"                            // ; Should call the exit system call defined in stdlib.h
        :::
    );
}
