AS=as -Iinclude
LD=ld
CC=gcc
CPP=gcc -E -nostdinc -Iinclude
CFLAGS=-Wall -pedantic -W -nostdlib -nostdinc -Wno-long-long -I include -fomit-frame-pointer

KERNEL_OBJS= load.o init.o isr.o timer.o libcc.o scr.o kb.o task.o kprintf.o hd.o exceptions.o fs.o mm.o

#.c.s:
#	${CC} ${CFLAGS} -S -o $*.s $<

.s.o:
	${AS} -a $< -o $*.o >$*.map

all: final.img

final.img: bootsect kernel
	cat bootsect kernel > final.img
	@wc -c final.img

bootsect: bootsect.o
	${LD} --oformat binary -N -e start -Ttext 0x7c00 -o bootsect $<

kernel: ${KERNEL_OBJS}
	${LD} --oformat binary -N -e pm_mode -Ttext 0x0000 -o $@ ${KERNEL_OBJS}
	@wc -c kernel

clean:
	rm -f *.img *.map kernel bootsect *.o

dep:
	sed '/\#\#\# Dependencies/q' < Makefile > tmp_make
	(for i in *.c;do ${CPP} -M $$i;done) >> tmp_make
	mv tmp_make Makefile

### Dependencies:
exceptions.o: exceptions.c include/kprintf.h include/scr.h include/asm.h \
  include/task.h include/kernel.h
fs.o: fs.c include/fs.h include/hd.h include/kprintf.h include/scr.h \
  include/kernel.h include/asm.h include/libcc.h
hd.o: hd.c include/hd.h include/asm.h include/kprintf.h include/scr.h \
  include/kernel.h
init.o: init.c include/scr.h include/isr.h include/asm.h include/kernel.h \
  include/task.h include/libcc.h include/timer.h include/hd.h \
  include/kprintf.h include/kb.h include/fs.h include/mm.h
kb.o: kb.c include/asm.h include/scr.h
kprintf.o: kprintf.c include/scr.h include/asm.h include/kprintf.h
libcc.o: libcc.c include/libcc.h
mm.o: mm.c include/mm.h include/kprintf.h include/scr.h include/asm.h \
  include/kernel.h include/fs.h include/hd.h include/libcc.h
scr.o: scr.c include/asm.h include/scr.h include/libcc.h
task.o: task.c include/task.h include/kernel.h include/asm.h
timer.o: timer.c include/asm.h include/task.h include/kernel.h \
  include/scr.h include/kprintf.h
