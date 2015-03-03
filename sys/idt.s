.text
.intel_syntax noprefix
.globl idt_flush
.globl remap_pic
idt_flush:
   lidt [rdi]
   ret

remap_pic:
    mov al, 0x11
    out 0x20, al     # restart PIC1
    out 0xA0, al     # restart PIC2

    mov al, 0x20
    out 0x21, al     # PIC1 now starts at 32
    mov al, 0x28
    out 0xA1, al     # PIC2 now starts at 40

    mov al, 0x04
    out 0x21, al     # setup cascading
    mov al, 0x02
    out 0xA1, al

    mov al, 0x01
    out 0x21, al
    out 0xA1, al     # done!
    ret


.extern isr_common_stub
.extern irq_common_stub

.macro ISR_ERRCODE num
  .global isr_\num
  isr_\num:
    cli
    push 0
    push \num
    jmp isr_common_stub
.endm


.macro ISR_NOERRCODE num
   .global isr_\num
   isr_\num:
      cli
      push 0
      push \num
      jmp isr_common_stub
.endm

.macro IRQ port data_port
  .global irq_\port
  irq_\port:
    cli
    push 0
    push \data_port
    jmp irq_common_stub
.endm

ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8
ISR_NOERRCODE 9
ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_ERRCODE   12
ISR_ERRCODE   13
ISR_ERRCODE   14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_NOERRCODE 17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_NOERRCODE 21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_NOERRCODE 29
ISR_NOERRCODE 30
ISR_NOERRCODE 31

IRQ 0, 32
IRQ 1, 33
IRQ 2, 34
IRQ 3, 35
IRQ 4, 36
IRQ 5, 37
IRQ 6, 38
IRQ 7, 39
IRQ 8, 40
IRQ 9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47
