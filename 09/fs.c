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

unsigned int
alloc_blk(struct SUPER_BLOCK *sb) {
	unsigned int i = 0, j = 0, n = 0, m = 0;
	unsigned char sect[512] = {0};

	n = ABS_DMAP_BLK(*sb);
	for (; i<sb->sb_dmap_blks; ++i) {
		hd_rw(n, HD_READ, 1, sect);
		for (j=0; j<512*8; ++j) {
			if (testb(sect, j)) {
				++m;
			} else {			/* gotcha */
				setb(sect, j);
				if (m >= sb->sb_blocks)
					return 0;
				else {
					hd_rw(n, HD_WRITE, 1, sect);
					return ABS_BOOT_BLK(*sb) + m;
				}
			}
		}
		++n;
	}
	return 0;
}

void
free_blk(struct SUPER_BLOCK *sb, unsigned int n) {
	unsigned char sect[512] = {0};
	unsigned int t = (n-ABS_BOOT_BLK(*sb))/(512*8)+ABS_DMAP_BLK(*sb);
	hd_rw(t, HD_READ, 1, sect);
	clrb(sect, (n-ABS_BOOT_BLK(*sb))%(512*8));
	hd_rw(t, HD_WRITE, 1, sect);
}

static int
alloc_inode(struct SUPER_BLOCK *sb) {
	unsigned char sect[512] = {0};
	int i = 0;
	hd_rw(ABS_IMAP_BLK(*sb), HD_READ, 1, sect);
	for (; i<512; ++i) {
		if (! testb(sect, i)) {
			setb(sect, i);
			hd_rw(ABS_IMAP_BLK(*sb), HD_WRITE, 1, sect);
			break;
		}
	}
	return (i==512)?-1:i;
}

static void
free_inode(struct SUPER_BLOCK *sb, int n) {
	unsigned char sect[512] = {0};
	hd_rw(ABS_IMAP_BLK(*sb), HD_READ, 1, sect);
	clrb(sect, n);
	hd_rw(ABS_IMAP_BLK(*sb), HD_WRITE, 1, sect);
}

static struct INODE *
iget(struct SUPER_BLOCK *sb, struct INODE *inode, int n) {
	unsigned char sect[512] = {0};
	int i = n/INODES_PER_BLK;
	int j = n%INODES_PER_BLK;

	hd_rw(ABS_INODE_BLK(*sb)+i, HD_READ, 1, sect);
	memcpy(inode, sect+j*sizeof(struct INODE), sizeof(struct INODE));
	return inode;
}

static void
iput(struct SUPER_BLOCK *sb, struct INODE *inode, int n) {
	unsigned char sect[512] = {0};
	int i = n/INODES_PER_BLK;
	int j = n%INODES_PER_BLK;
	hd_rw(ABS_INODE_BLK(*sb)+i, HD_READ, 1, sect);
	memcpy(sect+j*sizeof(struct INODE), inode, sizeof(struct INODE));
	hd_rw(ABS_INODE_BLK(*sb)+i, HD_WRITE, 1, sect);
}

static struct INODE iroot = {FT_DIR, 2*sizeof(struct DIR_ENTRY), {0,}};

static void
stat(struct INODE *inode) {
	unsigned int i = 0;
	char sect[512] = {0};
	struct DIR_ENTRY *de;

	kprintf(KPL_DUMP, "======== stat / ========\n");
	switch (inode->i_mode) {
	case FT_NML:
		kprintf(KPL_DUMP, "File, ");
		break;
	case FT_DIR:
		kprintf(KPL_DUMP, "Dir,  ");
		break;
	default:
		kprintf(KPL_PANIC, "UNKNOWN FILE TYPE!!");
		halt();
	}
	kprintf(KPL_DUMP, "Size: %d, ", inode->i_size);
	kprintf(KPL_DUMP, "Blocks: ");
	for (; i<8; ++i)
		kprintf(KPL_DUMP, "%d, ", inode->i_block[i]);
	hd_rw(inode->i_block[0], HD_READ, 1, sect);
	switch (inode->i_mode) {
	case FT_DIR:
		kprintf(KPL_DUMP, "\nName\tINode\n");
		de = (struct DIR_ENTRY *)sect;
		for (i=0; i<inode->i_size/sizeof(struct DIR_ENTRY); ++i) {
			kprintf(KPL_DUMP, "%s\t%x\n", de[i].de_name, de[i].de_inode);
		}
		break;
	default:
		break;
	}
}

void
install_color(void) {
	struct SUPER_BLOCK sb;
	char sect[512] = {0};
	struct DIR_ENTRY *de = NULL;
	int inode = -1;
	struct INODE clnode;
	unsigned int blk = 0;
	unsigned char color[] = {0x57,0x56,0x53,0x83,0xec,0x08,0xc7,0x44,0x24,0x04,0x00,0x00,0x00,0x00,0x83,0x7c,0x24,0x04,0x0f,0x7f,0x2e,0xc7,0x04,0x24,0x00,0x00,0x00,0x00,0x83,0x3c,0x24,0x0f,0x7f,0x19,0x8b,0x74,0x24,0x04,0x8b,0x3c,0x24,0xbb,0x58,0x00,0x00,0x00,0xb8,0x00,0x00,0x00,0x00,0xcd,0x80,0x89,0xe0,0xff,0x00,0xeb,0xe1,0x8d,0x44,0x24,0x04,0xff,0x00,0xeb,0xcb,0xeb,0xfe};

	sb.sb_start = *(unsigned int *)(HD0_ADDR);
	hd_rw(ABS_SUPER_BLK(sb), HD_READ, 1, sect);
	memcpy(&sb, sect, sizeof(struct SUPER_BLOCK));

	inode = alloc_inode(&sb);
	assert(inode > 0);
	blk = alloc_blk(&sb);
	assert(blk != 0);
	clnode.i_block[0] = blk;
	hd_rw(blk, HD_WRITE, 1, color);
	clnode.i_mode = FT_NML;
	clnode.i_size = sizeof color;
	iput(&sb, &clnode, inode);

	hd_rw(iroot.i_block[0], HD_READ, 1, sect);
	de = &((struct DIR_ENTRY *)sect)[2];
	strcpy(de->de_name, "color");
	de->de_inode = inode;
	hd_rw(iroot.i_block[0], HD_WRITE, 1, sect);

	iget(&sb, &iroot, 0);
	iroot.i_size = 3*sizeof(struct DIR_ENTRY);
	iput(&sb, &iroot, 0);
}

void
load_color(void) {
	struct INODE inode;
	struct SUPER_BLOCK sb;
	char sect[512] = {0};

	sb.sb_start = *(unsigned int *)(HD0_ADDR);
	hd_rw(ABS_SUPER_BLK(sb), HD_READ, 1, sect);
	memcpy(&sb, sect, sizeof(struct SUPER_BLOCK));
	iget(&sb, &inode, 1);

	/* for simplicity, just load the first sector of color to 0x100000 */
	hd_rw(inode.i_block[0], HD_READ, 1, (void *)0x100000);
}

static void
check_root(void) {
	struct SUPER_BLOCK sb;
	unsigned char sect[512] = {0};
	struct DIR_ENTRY *de = NULL;

	sb.sb_start = *(unsigned int *)(HD0_ADDR);
	hd_rw(ABS_SUPER_BLK(sb), HD_READ, 1, sect);
	memcpy(&sb, sect, sizeof(struct SUPER_BLOCK));
	hd_rw(ABS_IMAP_BLK(sb), HD_READ, 1, sect);
	if (! testb(sect, 0)) {
		kprintf(KPL_DUMP, "/ has not been created, creating....\t\t\t\t\t  ");
		if (alloc_inode(&sb) != 0) {
			kprintf(KPL_PANIC, "\n/ must be inode 0!!!\n");
			halt();
		}
		iroot.i_block[0] = alloc_blk(&sb);
		iput(&sb, &iroot, 0);
		
		hd_rw(iroot.i_block[0], HD_READ, 1, sect);
		de = (struct DIR_ENTRY *)sect;
		strcpy(de->de_name, ".");
		de->de_inode = 0;
		++de;
		strcpy(de->de_name, "..");
		de->de_inode = -1;
		hd_rw(iroot.i_block[0], HD_WRITE, 1, sect);
		kprintf(KPL_DUMP, "[DONE]");
		if (iroot.i_size == 2*sizeof(struct DIR_ENTRY))
			install_color();
	}
	iget(&sb, &iroot, 0);
	hd_rw(iroot.i_block[0], HD_READ, 1, sect);
	de = (struct DIR_ENTRY *)sect;
	
	if ((strcmp(de[0].de_name, ".")) || (de[0].de_inode) ||
		(strcmp(de[1].de_name, "..")) || (de[1].de_inode) != -1) {
		kprintf(KPL_PANIC, "File system is corrupted!!!\n");
		halt();
	}
}

void
verify_dir(void) {
	unsigned char sect[512] = {0};
	unsigned int *q = (unsigned int *)(HD0_ADDR);
	struct INODE inode;
	struct SUPER_BLOCK sb;

	sb.sb_start = q[0];
	hd_rw(ABS_SUPER_BLK(sb), HD_READ, 1, sect);
	check_root();
	memcpy(&sb, sect, sizeof(struct SUPER_BLOCK));
	stat(iget(&sb, &inode, 0));
}
