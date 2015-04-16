/* Skelix by Xiaoming Mo (xiaoming.mo@skelix.org)
 * Licence: GPLv2 */
void
color(void) {
	int i, j;
	for (i=0; i<16; ++i)
		for (j=0; j<16; ++j)
			__asm__ ("int	$0x80"::"S"(i),"D"(j),"b"('X'),"a"(0));
	for (;;)
		;
}
