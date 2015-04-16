/* Skelix by Xiaoming Mo (xiaoming.mo@skelix.org)
 * Licence: GPLv2 */
#ifndef KPRINTF_H
#define KPRINTF_H

#include <scr.h>

enum KP_LEVEL {KPL_DUMP, KPL_PANIC};

void kprintf(enum KP_LEVEL, const char *fmt, ...);

#endif
