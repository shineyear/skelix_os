AS=as -Iinclude
LD=ld

KERNEL_OBJS= load.o

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
	rm -f *.img kernel bootsect *.o

