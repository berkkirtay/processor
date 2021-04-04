1c1
< 	.file	"cpu.c"
---
> 	.file	"test.c"
889a890,934
> 	.globl	main
> 	.type	main, @function
> main:
> .LFB8:
> 	.cfi_startproc
> 	endbr64
> 	pushq	%rbp
> 	.cfi_def_cfa_offset 16
> 	.cfi_offset 6, -16
> 	movq	%rsp, %rbp
> 	.cfi_def_cfa_register 6
> 	subq	$32, %rsp
> 	movq	%fs:40, %rax
> 	movq	%rax, -8(%rbp)
> 	xorl	%eax, %eax
> 	movw	$2048, -32(%rbp)
> 	movw	$16, -30(%rbp)
> 	movw	$2049, -28(%rbp)
> 	movw	$3, -26(%rbp)
> 	movw	$513, -24(%rbp)
> 	movw	$2304, -22(%rbp)
> 	movw	$3072, -20(%rbp)
> 	movw	$10, -18(%rbp)
> 	movw	$2305, -16(%rbp)
> 	movw	$4352, -14(%rbp)
> 	movw	$4096, -12(%rbp)
> 	movw	$3584, -10(%rbp)
> 	leaq	-32(%rbp), %rax
> 	movl	$12, %esi
> 	movq	%rax, %rdi
> 	call	initializeCPU
> 	movl	$0, %eax
> 	call	cpu
> 	movl	$0, %eax
> 	movq	-8(%rbp), %rdx
> 	xorq	%fs:40, %rdx
> 	je	.L39
> 	call	__stack_chk_fail@PLT
> .L39:
> 	leave
> 	.cfi_def_cfa 7, 8
> 	ret
> 	.cfi_endproc
> .LFE8:
> 	.size	main, .-main
