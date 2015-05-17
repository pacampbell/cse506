.data

.extern syscall_common_handler
.extern kstack_top
.extern global_rip
.extern global_sp
.extern get_current_task

.text
.globl syscall_entry
syscall_entry:
	# save %rsp into global_sp
	movq %rsp, global_sp
	# save %rcx into global_rip
	movq %rcx, global_rip
	# Push rbx onto the user stack
	pushq %rbx
	# push %r12 onto the user stack
	pushq %r12
	# Save the user stack pointer into rbx
	movq %rsp, %rbx
	# Save the rdi value into r12
	movq %rax, %r12
	# Switch to the kernel stack
	movq kstack_top, %rsp
	# Get the processes kernel stack
	callq get_current_task
	# DEBUG ME
	xchg %bx, %bx
	# Set the process kernel stack to be used
	movq 0xA8(%rax), %rsp 
	# put back the original rdi value
	movq %r12, %rax
	# Push the user stack onto the kernel stack
	pushq %rbx
	# Push save rcx and r11 onto the kernel stack
	pushq %rcx			# Return address
	pushq %r11			# Flags
	# Syscal handler arguments position
	# %%rax 	SYSCALL NUMBER
    # %%rdi		ARG1
    # %%rsi     ARG2
    # %%rdx     ARG3
    # %%r10     ARG4
    # %%r8      ARG5
	# %%r9      ARG6 
	# Push syscall arguments onto the stack
	# pushq %r9
	# push the arguments into the correct order
	pushq %r8
	pushq %r10
	pushq %rdx
	pushq %rsi
	pushq %rdi
	pushq %rax
	# Pop all the arguments into the function call registers
	# rdi, rsi, rdx, rcx, r8, r9
	popq %rdi			# NUM
	popq %rsi           # ARG1
	popq %rdx           # ARG2
	popq %rcx           # ARG3
	popq %r8            # ARG4
	popq %r9            # ARG5
	# Call the system call handler
	callq syscall_common_handler
	# Return value is now in %rax
	popq %r11		    # restore flags
	popq %rcx		    # restore the return address
	# Set back the userspace stack
	popq %rsp
	# pop off the r12 value
	popq %r12
	# put back the rbx value
	popq %rbx
	# Jump back to userspace
	sysretq
	# retq
