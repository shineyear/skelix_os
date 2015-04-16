# Skelix by Xiaoming Mo (xiaoming.mo@skelix.org)
# Licence: GPLv2
		.text
		.globl	pm_mode
		.include "kernel.inc"
		.org 0
pm_mode:
		movl	$DATA_SEL,%eax
		movw	%ax,	%ds
		movw	%ax,	%es
		movw	%ax,	%fs
		movw	%ax,	%gs
		movw	%ax,	%ss
		movl	$STACK_BOT,%esp

		cld
		movl	$0x10200,%esi
		movl	$0x200,	%edi
		movl	$(KERNEL_SECT-1)<<7,%ecx
		rep
		movsl

		call	init

