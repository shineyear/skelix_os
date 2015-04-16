/* Skelix by Xiaoming Mo (xiaoming.mo@skelix.org)
 * Licence: GPLv2 */
#include <kprintf.h>
#include <asm.h>
#include <scr.h>
#include <task.h>
#include <mm.h>

void
divide_error(void) {
	__asm__ ("pushl	%%eax;call	info"::"a"(KPL_PANIC));
	halt();
}

void
debug_exception(void) {
	__asm__ ("pushl	%%eax;call	info"::"a"(KPL_PANIC));
	halt();
}

void
breakpoint(void) {
	__asm__ ("pushl	%%eax;call	info"::"a"(KPL_PANIC));
	halt();
}

void
nmi(void) {
	__asm__ ("pushl	%%eax;call	info"::"a"(KPL_PANIC));
	halt();
}

void
overflow(void) {
	__asm__ ("pushl	%%eax;call	info"::"a"(KPL_PANIC));
	halt();
}

void
bounds_check(void) {
	__asm__ ("pushl	%%eax;call	info"::"a"(KPL_PANIC));
	halt();
}

void
invalid_opcode(void) {
	__asm__ ("pushl	%%eax;call	info"::"a"(KPL_PANIC));
	halt();
}

void
cop_not_avalid(void) {
	__asm__ ("pushl	%%eax;call	info"::"a"(KPL_PANIC));
	halt();
}

void
double_fault(void) {
	__asm__ ("pushl	%%eax;call	info"::"a"(KPL_PANIC));
	halt();
}

void
overrun(void) {
	__asm__ ("pushl	%%eax;call	info"::"a"(KPL_PANIC));
	halt();
}

void
invalid_tss(void) {
	__asm__ ("pushl	%%eax;call	info"::"a"(KPL_PANIC));
	halt();
}

void
seg_not_present(void) {
	__asm__ ("pushl	%%eax;call	info"::"a"(KPL_PANIC));
	halt();
}

void
stack_exception(void) {
	__asm__ ("pushl	%%eax;call	info"::"a"(KPL_PANIC));
	halt();
}

void
general_protection(void) {
	__asm__ ("pushl	%%eax;call	info"::"a"(KPL_PANIC));
	halt();
}

void
page_fault(void) {
	__asm__ ("pushl	%%eax;call	do_page_fault"::"a"(KPL_PANIC));
	halt();
}

void
reversed(void) {
	__asm__ ("pushl	%%eax;call	info"::"a"(KPL_PANIC));
	halt();
}

void
coprocessor_error(void) {
	__asm__ ("pushl	%%eax;call	info"::"a"(KPL_PANIC));
	halt();
}


void
info(enum KP_LEVEL kl, 
	 unsigned int ret_ip, unsigned int ss, unsigned int gs, unsigned int fs, 
	 unsigned int es, unsigned int ds, unsigned int edi, unsigned int esi, 
	 unsigned int ebp, unsigned int esp, unsigned int ebx, unsigned int edx, 
	 unsigned int ecx, unsigned int eax, unsigned int isr_nr, unsigned int err,
	 unsigned int eip, unsigned int cs, unsigned int eflags,
	 unsigned int old_esp, unsigned int old_ss) {
	static const char *exception_msg[] = {
		"DIVIDE ERROR",
		"DEBUG EXCEPTION",
		"BREAKPOINT",
		"NMI",
		"OVERFLOW",
		"BOUNDS CHECK",
		"INVALID OPCODE",
		"COPROCESSOR NOT VALID",
		"DOUBLE FAULT",
		"OVERRUN",
		"INVALID TSS",
		"SEGMENTATION NOT PRESENT",
		"STACK EXCEPTION",
		"GENERAL PROTECTION",
		"PAGE FAULT",
		"REVERSED",
		"COPROCESSOR_ERROR",
	};
	unsigned int cr2, cr3;
	(void)ret_ip;
	__asm__ __volatile__ ("movl	%%cr2,	%%eax":"=a"(cr2));
	__asm__ __volatile__ ("movl %%cr3,	%%eax":"=a"(cr3));
	if (isr_nr < sizeof exception_msg)
		kprintf(kl, "EXCEPTION %d: %s\n=======================\n",
				isr_nr, exception_msg[isr_nr]);
	else
		kprintf(kl, "INTERRUPT %d\n=======================\n", isr_nr);
	kprintf(kl, "cs:\t%x\teip:\t%x\teflags:\t%x\n", cs, eip, eflags);
	kprintf(kl, "ss:\t%x\tesp:\t%x\n", ss, esp);
	kprintf(kl, "old ss:\t%x\told esp:%x\n", old_ss, old_esp);
	kprintf(kl, "errcode:%x\tcr2:\t%x\tcr3:\t%x\n", err, cr2, cr3);
	kprintf(kl, "General Registers:\n=======================\n");
	kprintf(kl, "eax:\t%x\tebx:\t%x\n", eax, ebx);
	kprintf(kl, "ecx:\t%x\tedx:\t%x\n", ecx, edx);
	kprintf(kl, "esi:\t%x\tedi:\t%x\tebp:\t%x\n", esi, edi, ebp);
	kprintf(kl, "Segment Registers:\n=======================\n");
	kprintf(kl, "ds:\t%x\tes:\t%x\n", ds, es);
	kprintf(kl, "fs:\t%x\tgs:\t%x\n", fs, gs);
}

void
print_task(struct TASK_STRUCT *task) {
	kprintf(0, "================ tss ===============\n");
	kprintf(0, "back_link:%x\tesp0:%x\t%ss0:%x\n", task->tss.back_link, 
			task->tss.esp0, task->tss.ss0);
	kprintf(0, "esp1:%x\tss1:%x\tesp2:%x\tss2:%x\n", task->tss.esp1, 
			task->tss.ss1, task->tss.esp2, task->tss.ss2);
	kprintf(0, "cr3:%x\teip:%x\teflags:%x\n", task->tss.cr3, 
			task->tss.eip, task->tss.eflags);
	kprintf(0, "eax:%x\tebx:%x\tecx:%x\tedx:%x\n", task->tss.eax,
			task->tss.ebx, task->tss.ecx, task->tss.edx);
	kprintf(0, "esp:%x\tebp:%x\tesi:%x\tedi:%x\n", task->tss.esp, 
			task->tss.ebp, task->tss.esi, task->tss.edi);
	kprintf(0, "es:%x\tcs:%x\tds:%x\n", task->tss.es, task->tss.cs,
			task->tss.ds);
	kprintf(0, "ss:%x\tfs:%x\tgs:%x\n", task->tss.ss, task->tss.fs,
			task->tss.gs);
	kprintf(0, "ldt:%x\ttrace_bitmap:%x\n", task->tss.ldt, 
			task->tss.trace_bitmap);
	kprintf(0, "=============== process ============\n");
	kprintf(0, "\ntss_entry:%x\tldt_entry:%x\n", task->tss_entry,
			task->ldt_entry);
	kprintf(0, "ldt[0]:%x %x\t\nldt[1]:%x %x\n", task->ldt[0], task->ldt[0],
			task->ldt[1], task->ldt[1]);
	kprintf(0, "state:%d\tpriority:%x\tnext:\n", task->state, task->priority, 
			task->next);
}
