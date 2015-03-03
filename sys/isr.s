.text
.intel_syntax noprefix
.extern isr_handler
.extern irq_handler


.globl isr_common_stub
isr_common_stub:
   # push all registers onto the stack
   push rdi
   push rsi
   push rbp
   push rsp
   push rbx
   push rdx
   push rcx
   push rax

   mov rax, ds
   push rax

   mov rax, 0x10   # load the kernel data segment
   mov ds, rax
   mov es, rax
   mov fs, rax
   mov gs, rax

   call isr_handler  # Defined in isr.c

   # reload the original data segment descriptor
   pop rax
   mov ds, rax
   mov es, rax
   mov fs, rax
   mov gs, rax

   # pop all registers off the stack
   pop rax
   pop rcx
   pop rdx
   pop rbx
   pop rsp
   pop rbp
   pop rsi
   pop rdi

   # re-enable interrupts
   sti
   add rsp, 16
   # Return from the interrupt
   iretq

.globl irq_common_stub
irq_common_stub:
  # push all registers onto the stack
  push rdi
  push rsi
  push rbp
  push rsp
  push rbx
  push rdx
  push rcx
  push rax

  mov rax, ds
  push rax

  mov rax, 0x10   # load the kernel data segment
  mov ds, rax
  mov es, rax
  mov fs, rax
  mov gs, rax

  call irq_handler  # Defined in isr.c

  # reload the original data segment descriptor
  pop rax
  mov ds, rax
  mov es, rax
  mov fs, rax
  mov gs, rax

  # pop all registers off the stack
  pop rax
  pop rcx
  pop rdx
  pop rbx
  pop rsp
  pop rbp
  pop rsi
  pop rdi

  # re-enable interrupts
  sti
  add rsp, 16
  # Return from the interrupt
  iretq
