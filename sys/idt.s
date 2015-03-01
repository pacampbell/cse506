.text
.intel_syntax noprefix
.globl idt_flush
idt_flush:
   lidt [rdi]
   ret


.extern isr_common_stub
.globl isr_0
isr_0:
   cli                        # disable interrupts
   push 0
   push 0
   jmp isr_common_stub        # go to a common handler


.globl isr_1
isr_1:
   cli                        # disable interrupts
   push 0
   push 1
   jmp isr_common_stub        # go to a common handler
