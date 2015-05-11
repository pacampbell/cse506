.text
.intel_syntax noprefix
.extern isr_handler
.extern irq_handler


.globl isr_common_stub
isr_common_stub:
   # push all registers onto the stack
   pushq rax
   pushq rbx
   pushq rcx
   pushq rdx
   pushq rbp
   pushq rsi
   pushq rdi
   pushq r8
   pushq r9
   pushq r10
   pushq r11
   pushq r12
   pushq r13
   pushq r14
   pushq r15

   # Save the ds value
   movq rax, ds
   pushq rax

   # load the kernel data segment
   movq rax, 0x10   
   movq ds, rax
   movq es, rax
   movq fs, rax
   movq gs, rax

   callq isr_handler  # Defined in isr.c

   # reload the original data segment descriptor
   popq rax
   movq ds, rax
   movq es, rax
   movq fs, rax
   movq gs, rax
   
   # pop all registers off the stack
   popq r15
   popq r14
   popq r13
   popq r12
   popq r11
   popq r10
   popq r9
   popq r8
   popq rdi
   popq rsi
   popq rbp
   popq rdx
   popq rcx
   popq rbx
   popq rax

   # re-enable interrupts
   addq rsp, 0x18
   sti
   # Return from the interrupt
   iretq

.globl irq_common_stub
irq_common_stub:
  # push all registers onto the stack
  pushq rax
  pushq rbx
  pushq rcx
  pushq rdx
  pushq rbp
  pushq rsi
  pushq rdi
  pushq r8
  pushq r9
  pushq r10
  pushq r11
  pushq r12
  pushq r13
  pushq r14
  pushq r15

  movq rax, ds
  pushq rax

  movq rax, 0x10   # load the kernel data segment
  movq ds, rax
  movq es, rax
  movq fs, rax
  movq gs, rax

  callq irq_handler  # Defined in isr.c

  # reload the original data segment descriptor
  popq rax
  movq ds, rax
  movq es, rax
  movq fs, rax
  movq gs, rax

  # pop all registers off the stack
  popq r15
  popq r14
  popq r13
  popq r12
  popq r11
  popq r10
  popq r9
  popq r8
  popq rdi
  popq rsi
  popq rbp
  popq rdx
  popq rcx
  popq rbx
  popq rax

  # re-enable interrupts
  addq rsp, 0x10
  sti
  # Return from the interrupt
  iretq
