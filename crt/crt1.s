.text
.intel_syntax noprefix
.globl _start
_start:
    mov rbp, rsp                     # rbp = rsp
    mov rdi, [rsp]                   # This should be argc

    mov rax, rsp                     # copy the stack pointer
    add rax, 0x8                     # add 8 to the offset
    mov rsi, rax                     # This should be argv[0]

    mov rdx, rax                     # store argv[0]
    mov rax, rdi                     # get argc
    imul rax, 0x8                    # argc * 8
    add rax, 0x8                     # (argc * 8) + 8
    add rdx, rax                     # This should be envp[0]

    call main                        # call main
    mov rdi, rax                     # Set exits first param to be the return value of main
    call exit                        # Should call the exit system call defined in stdlib.h
