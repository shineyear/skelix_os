/* Skelix by Xiaoming Mo (xiaoming.mo@skelix.org)
 * Licence: GPLv2 */
#ifndef MM_H
#define MM_H

#include <kprintf.h>

#define PG_UNUSED	0
#define PG_NORMAL	1
#define PG_TASK		2
#define PG_REVERSED	(-1)

void mm_install(void);
unsigned int alloc_page(int type);
void *page2mem(unsigned int nr);
void do_page_fault(enum KP_LEVEL kl,
				   unsigned int ret_ip, unsigned int ss, unsigned int gs,
				   unsigned int fs, unsigned int es, unsigned int ds, 
				   unsigned int edi, unsigned int esi, unsigned int ebp,
				   unsigned int esp, unsigned int ebx, unsigned int edx, 
				   unsigned int ecx, unsigned int eax, unsigned int isr_nr, 
				   unsigned int err, unsigned int eip, unsigned int cs, 
				   unsigned int eflags,unsigned int old_esp, 
				   unsigned int old_ss);

#endif
