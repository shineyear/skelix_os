AS=as
LD=ld

.s.o:
	${AS} -a $< -o $*.o >$*.map

all: final.img

final.img: bootsect
	mv bootsect final.img

bootsect: bootsect.o
	${LD} --oformat binary -N -e start -Ttext 0x7c00 -o bootsect $<

