/* Skelix by Xiaoming Mo (xiaoming.mo@skelix.org)
 * Licence: GPLv2 */
#include <fs.h>
#include <hd.h>
#include <kprintf.h>
#include <kernel.h>
#include <asm.h>
#include <libcc.h>

void
setb(void *s, unsigned int i) {
	unsigned char *v = s;
	v += i>>3;
	*v |= 1<<(7-(i%8));
}

void
clrb(void *s, unsigned int i) {
	unsigned char *v = s;
	v += i>>3;
	*v &= ~(1<<(7-(i%8)));
}

int
testb(void *s, unsigned int i) {
	unsigned char *v = s;
	v += i>>3;
	return (*v&(1<<(7-(i%8)))) !=0;
}

void
verify_fs(void) {
	unsigned int *q = (unsigned int *)(HD0_ADDR);
	unsigned char sect[512] = {0};
	struct SUPER_BLOCK sb;
	unsigned int i = 0, j = 0, m = 0, n = 0;

	/* get the fs sb */
	sb.sb_start = q[0];
	hd_rw(ABS_SUPER_BLK(sb), HD_READ, 1, &sb);
	/* the first partition must be FST_FS type */
	if (sb.sb_magic != FST_FS) {
		kprintf(KPL_DUMP, "Partition 1 does not have a valid file system\n");
		kprintf(KPL_DUMP, "Creating file system\t\t\t\t\t\t\t  ");
		sb.sb_magic = FST_FS;
		sb.sb_start = q[0];
		sb.sb_blocks = q[1];
		sb.sb_dmap_blks = (sb.sb_blocks+0xfff)>>12;
		sb.sb_imap_blks = INODE_BIT_BLKS;
		sb.sb_inode_blks = INODE_BLKS;
		hd_rw(ABS_SUPER_BLK(sb), HD_WRITE, 1, &sb);

		/* initial data bitmap */
		n = ABS_DMAP_BLK(sb);
		j = sb.sb_dmap_blks+sb.sb_imap_blks+sb.sb_inode_blks+2;
		memset(sect, 0xff, sizeof sect/sizeof sect[0]);
		for (i=j/(512*8); i>0; --i) {
			hd_rw(n++, HD_WRITE, 1, sect);
			m += 4096;
		}
		m += 4096;
		for (i=j%(512*8); i<512*8; ++i) {
			clrb(sect, i);
			--m;
		}
		hd_rw(n++, HD_WRITE, 1, sect);

		memset(sect, 0, sizeof sect/sizeof sect[0]);
		for (i=sb.sb_imap_blks-(n-ABS_DMAP_BLK(sb)); i>0; --i)
			hd_rw(n++, HD_WRITE, 1, sect);

		/* initail inode bitmap */
		for (i=sb.sb_imap_blks; i>0; --i)
			hd_rw(ABS_IMAP_BLK(sb)+i-1, HD_WRITE, 1, sect);

		/* initial inode blocks */
		for (i=sb.sb_inode_blks; i>0; --i)
			hd_rw(ABS_INODE_BLK(sb)+i-1, HD_WRITE, 1, sect);
		kprintf(KPL_DUMP, "[DONE]");
	}
	q += 2;

	kprintf(KPL_DUMP, "0: Type: FST_FS ");
	kprintf(KPL_DUMP, "start at: %d ", sb.sb_start);
	kprintf(KPL_DUMP, "blocks: %d ", sb.sb_blocks);
	kprintf(KPL_DUMP, "dmap: %d ", sb.sb_dmap_blks);
	kprintf(KPL_DUMP, "imap: %d ", sb.sb_imap_blks);
	kprintf(KPL_DUMP, "inodes: %d\n", sb.sb_inode_blks);

	/* initialize swap partition */
	sb.sb_start = q[0];
	hd_rw(ABS_SUPER_BLK(sb), HD_READ, 1, &sb);
	if (sb.sb_magic != FST_SW) {
		kprintf(KPL_DUMP, "\nPartition 2 does not have a valid file system\n");
		kprintf(KPL_DUMP, "Creating file system\t\t\t\t\t\t\t  ");
		sb.sb_magic = FST_SW;
		sb.sb_start = q[0];
		sb.sb_blocks = q[1];
		sb.sb_dmap_blks = (sb.sb_blocks)>>15;	/* 1 bits == 4K page */
		hd_rw(ABS_SUPER_BLK(sb), HD_WRITE, 1, &sb);
		kprintf(KPL_DUMP, "[DONE]");	
	}
	/* initial data bitmap */
	n = ABS_DMAP_BLK(sb);
	j = sb.sb_dmap_blks+2;
	memset(sect, 0xff, sizeof sect/sizeof sect[0]);
	for (i=j/(512*8); i>0; --i) {
		hd_rw(n++, HD_WRITE, 1, sect);
		m += 4096;
	}
	m += 4096;
	for (i=j%(512*8); i<512*8; ++i) {
		clrb(sect, i);
		--m;
	}
	hd_rw(n++, HD_WRITE, 1, sect);

	kprintf(KPL_DUMP, "1: Type: FST_SW ");
	kprintf(KPL_DUMP, "start at: %d ", sb.sb_start);
	kprintf(KPL_DUMP, "blocks: %d ", sb.sb_blocks);
	kprintf(KPL_DUMP, "dmap: %d, presents %d 4k-page\n",
			sb.sb_dmap_blks, sb.sb_blocks>>3);
}

