	.file	"color.c"
	.text
.globl color
	.type	color, @function
color:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx
	subl	$8, %esp
	movl	$0, -16(%ebp)
.L2:
	cmpl	$15, -16(%ebp)
	jg	.L3
	movl	$0, -20(%ebp)
.L5:
	cmpl	$15, -20(%ebp)
	jg	.L4
	movl	-16(%ebp), %esi
	movl	-20(%ebp), %edi
	movl	$88, %ebx
	movl	$0, %eax
#APP
	int	$0x80
#NO_APP
	leal	-16(%ebp), %eax
	incl	(%eax)
	jmp	.L5
.L4:
	leal	-16(%ebp), %eax
	incl	(%eax)
	jmp	.L2
.L3:
.L8:
	jmp	.L8
	.size	color, .-color
	.section	.note.GNU-stack,"",@progbits
	.ident	"GCC: (GNU) 3.4.6 (Gentoo 3.4.6-r1, ssp-3.4.5-1.0, pie-8.7.9)"
