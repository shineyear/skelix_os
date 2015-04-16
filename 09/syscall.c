/* Skelix by Xiaoming Mo (xiaoming.mo@skelix.org)
 * Licence: GPLv2 */
#include <isr.h>
#include <syscall.h>

void (*sys_call_table[VALID_SYSCALL])(void) = {sys_print};

