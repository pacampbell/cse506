.text
.intel_syntax noprefix
.extern isr_handler
.extern irq_handler


.globl isr_common_stub
isr_common_stub:
   # push all registers onto the stack
   push rax
   push rbx
   push rcx
   push rdx
   push rbp
   push rsi
   push rdi
   push r8
   push r9
   push r10
   push r11
   push r12
   push r13
   push r14
   push r15

   # Save the ds value
   mov rax, ds
   push rax

   # load the kernel data segment
   mov rax, 0x10   
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
   pop r15
   pop r14
   pop r13
   pop r12
   pop r11
   pop r10
   pop r9
   pop r8
   pop rdi
   pop rsi
   pop rbp
   pop rdx
   pop rcx
   pop rbx
   pop rax

   # re-enable interrupts
   add rsp, 0x18
   # sti
   # Return from the interrupt
   iretq

.globl irq_common_stub
irq_common_stub:
  # push all registers onto the stack
  push rax
  push rbx
  push rcx
  push rdx
  push rbp
  push rsi
  push rdi
  push r8
  push r9
  push r10
  push r11
  push r12
  push r13
  push r14
  push r15

  mov rax, ds
  push rax

  mov rax, 0x10   # load the kernel data segment
  mov ds, rax
  mov es, rax
  mov fs, rax
  mov gs, rax

  # mov rdi, 0x1234
  call irq_handler  # Defined in isr.c

  # reload the original data segment descriptor
  pop rax
  mov ds, rax
  mov es, rax
  mov fs, rax
  mov gs, rax

  # pop all registers off the stack
  pop r15
  pop r14
  pop r13
  pop r12
  pop r11
  pop r10
  pop r9
  pop r8
  pop rdi
  pop rsi
  pop rbp
  pop rdx
  pop rcx
  pop rbx
  pop rax

  # re-enable interrupts
  add rsp, 0x10
  # sti
  # Return from the interrupt
  iretq
