/* Skelix by Xiaoming Mo (xiaoming.mo@skelix.org)
 * Licence: GPLv2 */
#include <mm.h>
#include <kprintf.h>
#include <asm.h>
#include <kernel.h>
#include <fs.h>
#include <hd.h>
#include <libcc.h>

static unsigned char mmap[MEMORY_RANGE/PAGE_SIZE] = {PG_REVERSED, };
#ifndef VM_BUG_FREE
void
mm_install(void) {
	unsigned long *page_dir = ((unsigned long *)PAGE_DIR);
	unsigned long *page_table = ((unsigned long *)PAGE_TABLE);
	unsigned long address = 0;
	int i;
	for(i=0; i<MEMORY_RANGE/(4*1024); ++i) {
		/* attribute set to: kernel, r/w, present */
		page_table[i] = address|7;
		address += 4*1024;
	};
	page_dir[0] = ((unsigned long)page_table|7);
	for (i=1; i<1023; ++i)
		page_dir[i] = 6;
	__asm__ __volatile__ (
		"movl	%%eax,	%%cr3\n\t"
		"movl	%%cr0,	%%eax\n\t"
		"orl	$0x80000000,	%%eax\n\t"
		"movl	%%eax,	%%cr0"::"eax"((unsigned long)page_dir));

	/* set lower 1MB memory to used */
	for (i=(1*1024*1024)/(4*1024); i>=0; --i)
		mmap[i] = 0xff;
}
#else
void
mm_install(void) {
	unsigned long *page_dir = ((unsigned long *)PAGE_DIR);
	unsigned long *page_table = ((unsigned long *)PAGE_TABLE);
	unsigned long address = 0;
	int i;
	for(i=0; i<1024; ++i) {
		/* attribute set to: kernel, r/w, present */
		page_table[i] = address|7;
		address += PAGE_SIZE;
	};
	page_dir[0] = (PAGE_TABLE|7);
	for (i=1; i<1024; ++i)
		page_dir[i] = 6;
	/* set lower 1MB memory to used */
	for (i=(1*1024*1024)/(4*1024)-1; i>=0; --i)
		mmap[i] = PG_REVERSED;
	__asm__ __volatile__ (
		"movl	%%eax,	%%cr3\n\t"
		"movl	%%cr0,	%%eax\n\t"
		"orl	$0x80000000,	%%eax\n\t"
		"movl	%%eax,	%%cr0"::"a"(PAGE_DIR));
}
#endif

unsigned int
alloc_page(int type) {
	int i;

	cli();
	for (i=(sizeof mmap)-1; i>=0 && mmap[i]; --i)
		;

	if (i < 0) {
		unsigned char sect[512] = {0};
		unsigned int *q = (unsigned int *)(HD0_ADDR);
		struct SUPER_BLOCK sb;
		unsigned int j;

		for (i=(sizeof mmap)-1; i>=0; --i) {
			if (i == PG_NORMAL)
				break;
		}

		if ((((unsigned int *)PAGE_TABLE)[i] & (1<<6))) { /* dirty */
			sb.sb_start = q[3];
			hd_rw(ABS_SUPER_BLK(sb), HD_READ, 1, sect);
			memcpy(&sb, sect, sizeof(struct SUPER_BLOCK));
			j = alloc_blk(&sb);	/* get one free page on disk */
			if (j == 0) {
				kprintf(1, "NO PAGES CAN BE SWAPPED OUT");
				halt();
			}
			hd_rw(j, HD_WRITE, PAGE_SIZE/512, page_to_mem(i));
			((unsigned int *)PAGE_TABLE)[i] = (j<<12)|(1<<11)|6;
		} 
	}
	mmap[i] = type;
	sti();
	return i;
}

void *
page_to_mem(unsigned int nr) {
	return (void *)(nr * PAGE_SIZE);
}

void
do_page_fault(enum KP_LEVEL kl,
			  unsigned int ret_ip, unsigned int ss, unsigned int gs,
			  unsigned int fs, unsigned int es, unsigned int ds, 
			  unsigned int edi, unsigned int esi, unsigned int ebp,
			  unsigned int esp, unsigned int ebx, unsigned int edx, 
			  unsigned int ecx, unsigned int eax, unsigned int isr_nr, 
			  unsigned int err, unsigned int eip, unsigned int cs, 
			  unsigned int eflags,unsigned int old_esp, unsigned int old_ss) {
	unsigned int cr2, cr3;
	(void)ret_ip; (void)ss; (void)gs; (void)fs; (void)es; 
	(void)ds; (void)edi; (void)esi; (void)ebp; (void)esp; 
	(void) ebx; (void)edx; (void)ecx; (void)eax; 
	(void)isr_nr; (void)eip; (void)cs; (void)eflags; 
	(void)old_esp; (void)old_ss; (void)kl;
	__asm__ __volatile__ ("movl %%cr2, %%eax":"=a"(cr2));
	__asm__ __volatile__ ("movl %%cr3, %%eax":"=a"(cr3));
	kprintf(0, "\n  The fault at %x cr3:%x was caused by a %s. "
			"The accessing cause of the fault was a %s, when the "
			"processor was executing in %s mode.\n", 
			cr2, cr3,
			(err&0x1)?"page-level protection voilation":"not-present page", 
			(err&0x2)?"write":"read", 
			(err&0x4)?"user":"supervisor");

#if 0
	if (! (err&0x1)) {
		int i = alloc_page(PG_NORMAL);
			kprintf(0, "page 0x%x is free", i);
		if ((((unsigned int *)PAGE_TABLE)[cr2/PAGE_SIZE]&(1<<11)) == 0) {
			kprintf(1, "not on disk");
			unsigned int *pd_entry = &((unsigned int *)PAGE_DIR)[cr2>>22];
			if (*pd_entry & 1) {
				*(unsigned int *)((*pd_entry)&0xfffffe00) = (i*4096)|7;
			} else {
				int j = alloc_page(PG_NORMAL);
				int k;
				*pd_entry = (j*4096)|7;
				for (k=0; k<1024; ++k)
					((unsigned int *)(j*4096))[k] = 6;
				((unsigned int *)(j*4096))[(cr2&0x3fffff)>>12] = (i*4096)|7;
			}
		} else {
			hd_rw(((unsigned int *)PAGE_TABLE)[cr2/PAGE_SIZE]>>12, 
				  HD_READ, 1, (void *)(cr2&0xfffffe00));
			((unsigned int *)PAGE_TABLE)[cr2/PAGE_SIZE] = (i<<12)|7;
		}
	}
	__asm__ ("movl	%%eax, %%cr3"::"a"(PAGE_DIR));
#endif
}
