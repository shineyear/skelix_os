/* Skelix by Xiaoming Mo (xiaoming.mo@skelix.org)
 * Licence: GPLv2 */
#ifndef FS_H
#define FS_H

#define FT_NML	1
#define FT_DIR	2

struct INODE {
	unsigned int i_mode;		/* file mode */
	unsigned int i_size;		/* size in bytes */
	unsigned int i_block[8];
};

extern struct INODE iroot;

#define MAX_NAME_LEN 11

struct DIR_ENTRY {
	char de_name[MAX_NAME_LEN];
	int de_inode;
};

#define ABS_BOOT_BLK(sb)		((sb).sb_start)
#define ABS_SUPER_BLK(sb)		((ABS_BOOT_BLK(sb))+1)
#define ABS_DMAP_BLK(sb)		((ABS_SUPER_BLK(sb))+1)
#define ABS_IMAP_BLK(sb)		((ABS_DMAP_BLK(sb))+(sb).sb_dmap_blks)
#define ABS_INODE_BLK(sb)		((ABS_IMAP_BLK(sb))+(sb).sb_imap_blks)
#define ABS_DATA_BLK(sb)		((ABS_INODE_BLK(sb))+INODE_BLKS)

#define INODE_BIT_BLKS		1	/* 512*8 = 4096 inodes */
#define INODES_PER_BLK		(512/sizeof(struct INODE))
#define INODE_BLKS		((INODE_BIT_BLKS*512*8+INODES_PER_BLK-1)/(INODES_PER_BLK))

struct SUPER_BLOCK {
	unsigned char sb_magic;
	/* DPT 0x08 */
	unsigned int sb_start;
	/* DPT 0x0c */
	unsigned int sb_blocks;

	unsigned int sb_dmap_blks;
	unsigned int sb_imap_blks;
	unsigned int sb_inode_blks;
};

#define FST_FS	0x2e			/* normal partition */
#define FST_SW	0x2f			/* swap partition */

void verify_fs(void);

#endif
