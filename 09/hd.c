/* Skelix by Xiaoming Mo (xiaoming.mo@skelix.org)
 * Licence: GPLv2 */
#include <hd.h>
#include <asm.h>
#include <kprintf.h>
#include <kernel.h>
#include <fs.h>

struct HD_PARAM {
	unsigned int cyl;
	unsigned int head;
	unsigned int sect;
} HD0 = {208, 16, 63};

/* lba:	starts from 0 
 start_sect: starts from 1 */
void
hd_rw(unsigned int lba, unsigned int com,
	  unsigned int sects_to_access, void *buf) {
	/* lba to chs */
	/* cylinder = LBA / (heads_per_cylinder * sectors_per_track)
	   temp = LBA % (heads_per_cylinder * sectors_per_track)
	   head = temp / sectors_per_track
	   sector = temp % sectors_per_track + 1 */
	unsigned int cyl = lba/(HD0.head * HD0.sect);
	unsigned int head = (lba%(HD0.head*HD0.sect))/HD0.sect;
	unsigned int sect = (lba%(HD0.head*HD0.sect))%HD0.sect+1;

#ifdef HD_DEBUG
	if (com == HD_WRITE)
		kprintf(1, "[w");
	else
		kprintf(1, "[r");
	kprintf(1, "%d]", lba);
#endif
	while ((inb(HD_PORT_STATUS)&0xc0)!=0x40)
		;
	outb(sects_to_access, HD_PORT_SECT_COUNT);
	outb(sect, HD_PORT_SECT_NUM);
	outb(cyl, HD_PORT_CYL_LOW);
	outb(cyl>>8, HD_PORT_CYL_HIGH);
	outb(0xa0|head, HD_PORT_DRV_HEAD);
	outb(com, HD_PORT_COMMAND);
	while (! (inb(HD_PORT_STATUS)&0x8))
		;
	if (com == HD_READ)
		insl(HD_PORT_DATA, buf, sects_to_access<<7);
	else if (com == HD_WRITE)
		outsl(buf, sects_to_access<<7, HD_PORT_DATA);
}

static void
setup_DPT(void) {
	unsigned char sect[512] = {0};
	sect[0x1be] = 0x80;
	sect[0x1be + 0x04] = FST_FS;
	*(unsigned long *)&sect[0x1be + 0x08] = 1;
	*(unsigned long *)&sect[0x1be + 0x0c] = 85*1024*2; /* 85MB */
	sect[0x1ce + 0x04] = FST_SW;
	*(unsigned long *)&sect[0x1ce + 0x08] = 85*1024*2+1;
	*(unsigned long *)&sect[0x1ce + 0x0c] = 16*1024*2; /* 16MB */
	sect[0x1fe] = 0x55;
	sect[0x1ff] = 0xaa;
	hd_rw(0, HD_WRITE, 1, sect);
}

void
verify_DPT(void) {
	unsigned char sect[512];
	unsigned i = 0;
	unsigned int *q = (unsigned int *)(HD0_ADDR);

	hd_rw(0, HD_READ, 1, sect);
	if ((sect[0x1fe]==0x55) && (sect[0x1ff]==0xaa)) {
		unsigned char *p = &sect[0x1be];
		char *s;
		kprintf(KPL_DUMP, "   | Bootable | Type      | Start Sector | Capacity \n");
		for (i=0; i<4; ++i) {
			kprintf(KPL_DUMP, " %d ", i);
			/* system indicator at offset 0x04 */
			if (p[0x04] == 0x00) {
				kprintf(KPL_DUMP, "| Empty\n");
				p += 16;
				q += 2;
				continue;
			}
			if (p[0x00] == 0x80)
				s = "| Yes      ";
			else
				s = "| No       ";
			kprintf(KPL_DUMP, s);
			/* system indicator at offset 0x04 */
			if (p[0x04] == FST_FS) {
				kprintf(KPL_DUMP, "| Skelix FS ");
			} else if (p[0x04] == FST_SW) {
				kprintf(KPL_DUMP, "| Skelix SW ");
			} else
				kprintf(KPL_DUMP, "| Unknown   ", *p);
			/* starting sector number */
			*q++ = *(unsigned long *)&p[0x08];
			kprintf(KPL_DUMP, "| 0x%x   ", *(unsigned long*)&p[0x08]);
			/* capacity */
			*q++ = *(unsigned long*)&p[0x0c];
			kprintf(KPL_DUMP, "| %dM\n", (*(unsigned long*)&p[0x0c]*512)>>20);
			p += 16;
		}
	} else {
		kprintf(KPL_DUMP, "No bootable DPT found on HD0\n");
		kprintf(KPL_DUMP, "Creating DPT on HD0 automaticly\n");
		kprintf(KPL_DUMP, "Creating file system whatever you want it or not!!\n");
		setup_DPT();
		verify_DPT();
	}
}
