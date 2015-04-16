/* Skelix by Xiaoming Mo (xiaoming.mo@skelix.org)
 * Licence: GPLv2 */
#include <scr.h>
#include <isr.h>
#include <asm.h>
#include <kernel.h>
#include <task.h>
#include <libcc.h>
#include <timer.h>
#include <hd.h>
#include <fs.h>
#include <kprintf.h>
#include <mm.h>
#include <kb.h>

unsigned long long *idt = ((unsigned long long *)IDT_ADDR);
unsigned long long *gdt = ((unsigned long long *)GDT_ADDR);

static void 
isr_entry(int index, unsigned long long offset) {
	unsigned long long idt_entry = 0x00008e0000000000ULL |
			((unsigned long long)CODE_SEL<<16);
	idt_entry |= (offset<<32) & 0xffff000000000000ULL;
	idt_entry |= (offset) & 0xffff;
	idt[index] = idt_entry;
}

static void
sys_call_install(void) {
	unsigned long long sys_call_entry = 0x0000ef0000080000ULL |
			((unsigned long long)CODE_SEL<<16);
	sys_call_entry |= ((unsigned long long)sys_call<<32) & 
		0xffff000000000000ULL;
	sys_call_entry |= ((unsigned long long)sys_call) & 0xffff;
	idt[SYS_CALL] = sys_call_entry;
}

static void 
idt_install(void) {
	unsigned int i = 0;
	struct DESCR {
		unsigned short length;
		unsigned long address;
	} __attribute__((packed)) idt_descr = {256*8-1, IDT_ADDR};
	for (i=0; i<VALID_ISR; ++i)
		isr_entry(i, (unsigned int)(isr[(i<<1)+1]));
	for (++i; i<256; ++i)
		isr_entry(i, (unsigned int)default_isr);
	__asm__ ("lidt	%0\n\t"::"m"(idt_descr));
}

static void 
pic_install(void) {
	outb(0x11, 0x20);
	outb(0x11, 0xa0);
	outb(0x20, 0x21);
	outb(0x28, 0xa1);
	outb(0x04, 0x21);
	outb(0x02, 0xa1);
	outb(0x01, 0x21);
	outb(0x01, 0xa1);
	outb(0xff, 0x21);
	outb(0xff, 0xa1);
}

extern void task1_run(void);
extern void task2_run(void);
#ifndef VM_BUG_FREE
static void
new_task(long eip) {
	struct TASK_STRUCT *task = page_to_mem(alloc_page(PG_TASK));
	memcpy(task, &TASK0, sizeof(struct TASK_STRUCT));
	task->state = TS_STOPPED;
	task->tss.esp0 = (unsigned int)page_to_mem(alloc_page(PG_TASK))+PAGE_SIZE;
	task->tss.eip = eip;
	task->tss.eflags = 0x3202;
	task->tss.esp = (unsigned int)page_to_mem(alloc_page(PG_TASK))+PAGE_SIZE;
	task->priority = INITIAL_PRIO;
	task->next = current->next;
	current->next = task;
	task->state = TS_RUNABLE;
}
#else
static void
new_task(long eip) {
	int i;
	struct TASK_STRUCT *task = page_to_mem(alloc_page(PG_TASK));
	task->state = TS_STOPPED;
	memcpy(&task->tss, &(TASK0.tss), sizeof(struct TSS_STRUCT));
	task->tss.cr3 = (unsigned long)page_to_mem(alloc_page(PG_TASK));
	task->tss.esp0 = (unsigned long)page_to_mem(alloc_page(PG_TASK))+PAGE_SIZE;
	task->tss.eip = eip;
	task->tss.eflags = 0x3202;
	task->tss.esp = (unsigned long)page_to_mem(alloc_page(PG_NORMAL))+PAGE_SIZE;
	task->ldt[0] = DEFAULT_LDT_CODE;
	task->ldt[1] = DEFAULT_LDT_DATA;
	task->priority = INITIAL_PRIO;
	task->page_table = page_to_mem(alloc_page(PG_NORMAL));
	for (i=0; i<256; ++i)
		task->page_table[i] = (i*4096)|7;
	for (i=256; i<1024; ++i)
		task->page_table[i] = 0;

	((unsigned int *)task->tss.cr3)[0] = (unsigned int)task->page_table|7;
	for (i=1; i<1024; ++i)
		((unsigned int *)task->tss.cr3)[i] = 6;
	task->next = current->next;
	current->next = task;
	task->state = TS_RUNABLE;
}
#endif

void
do_task1(void) {
	__asm__ ("incb 0xb8000+160*24+2");
	load_color();
	__asm__ ("jmp 0x100000");
}

void
do_task2(void) {
	__asm__ ("incb 0xb8000+160*24+4");
}

void 
init(void) {
	char wheel[] = {'\\', '|', '/', '-'};
	int i = 0;

	idt_install();
	sys_call_install();
	pic_install();
	mm_install();
	kb_install();
	timer_install(100);

	set_tss((unsigned long long)&TASK0.tss);
	set_ldt((unsigned long long)&TASK0.ldt);
	__asm__ ("ltrw	%%ax\n\t"::"a"(TSS_SEL));
	__asm__ ("lldt	%%ax\n\t"::"a"(LDT_SEL));

	kprintf(KPL_DUMP, "Verifing disk partition table....\n");
	verify_DPT();
	kprintf(KPL_DUMP, "Verifing file systes....\n");
	verify_fs();
	kprintf(KPL_DUMP, "Checking / directory....\n");
	verify_dir();

	sti();

	new_task((long)task2_run);
	new_task((long)task1_run);

	__asm__ ("movl %%esp,%%eax\n\t" \
			 "pushl %%ecx\n\t" \
			 "pushl %%eax\n\t" \
			 "pushfl\n\t" \
			 "pushl %%ebx\n\t" \
			 "pushl $1f\n\t" \
			 "iret\n" \
			 "1:\tmovw %%cx,%%ds\n\t" \
			 "movw %%cx,%%es\n\t" \
			 "movw %%cx,%%fs\n\t" \
			 "movw %%cx,%%gs" \
			 ::"b"(USER_CODE_SEL),"c"(USER_DATA_SEL));
	
	for (;;) {
		__asm__ ("movb	%%al,	0xb8000+160*24"::"a"(wheel[i]));
		if (i == sizeof wheel)
			i = 0;
		else
			++i;
	}
}
