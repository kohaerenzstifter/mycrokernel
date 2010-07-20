VIDEO_SIZE = $$(stat -c %s video.bin)

VIDEO_HLA_SRC = go.hla $(shell ls *.hla | grep -v "go.hla")
VIDEO_HLA_OBJ = $(VIDEO_HLA_SRC:.hla=.o)

VIDEO_C_SRC = $(shell ls *.c)
VIDEO_C_OBJ = $(VIDEO_C_SRC:.c=.o)

VIDEO_OBJ = $(VIDEO_HLA_OBJ) $(VIDEO_C_OBJ) ../usrlib/syscall.o


HLA_HEADERS = $(shell ls *.hhf ../include/*.hhf)
C_HEADERS = $(shell ls *.h ../include/*.h)

all: video.bin

video.bin: $(VIDEO_OBJ) Makefile
	ld -melf_i386 $(VIDEO_OBJ) -N -M > video.mem -o video.bin --oformat binary -T ../script
	ndisasm -b 32 video.bin > video.ndisasm
	dd if=/dev/null of=video.bin seek=$$(( ($(VIDEO_SIZE) / 512) + 1 )) count=1

../usrlib/syscall.o:
	cd ../usrlib && make syscall.o

%.o: %.hla Makefile $(HLA_HEADERS)
	hla -c $<
	objdump --disassemble-all -S $@ > $*.objdump

%.o: %.c Makefile $(C_HEADERS)
	cc -m32 -Wall -fno-stack-protector -fno-leading-underscore -c $<
	objdump --disassemble-all -S $*.o > $*.objdump


clean:
	-rm *.bin
	-rm *.o
	-rm *.objdump
	-rm *.ndisasm
	-rm *.mem
	-rm *~
