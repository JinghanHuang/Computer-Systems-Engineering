
.text
.globl keyboard_handler, mouse_handler, rtc_handler, syscall_handler, pit_handler
.align 4
# assemble linkage for keyboard interrupt handler
keyboard_handler:
	pushal
	pushfl
	call keyboard_interrupt_handler
	popfl
	popal
	iret

# assemble linkage for mouse interrupt handler
mouse_handler:
	pushal
	pushfl
	call mouse_interrupt_handler
	popfl
	popal
	iret

# assemble linkage for rtc interrupt handler
rtc_handler:
	pushal
	pushfl
	call rtc_interrupt_handler
	popfl
	popal
	iret

# assemble linkage for pit interrupt handler
pit_handler:
	pushal
	pushfl
	call pit_interrupt_handler
	popfl
	popal
	iret
#entry 1-6 for check point 2; 0x0 first so that we don't need offset
syscal_jmp_tbl:
	.long 0x0, halt, execute, read, write, open, close, getargs, vidmap
# assemble linkage for system call handler
syscall_handler:
	pushfl

	pushl %ebp		#save all except eax
	pushl %edi
	pushl %esi

	pushl %edx		#following 3 are arguments
	pushl %ecx
	pushl %ebx

	cmpl $1, %eax	# make sure eax >= 1
	jl invalid
	cmpl $8, %eax	# make sure eax <= 8
	jg invalid

	call *syscal_jmp_tbl(,%eax,4)
	jmp go_back

invalid:
 	movl $-1, %eax	#return -1


go_back:
	popl %ebx
	popl %ecx
	popl %edx
	popl %esi
	popl %edi
	popl %ebp
	popfl

	iret
