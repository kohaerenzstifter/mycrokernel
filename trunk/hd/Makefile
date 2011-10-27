HD_SIZE = $$(stat -c %s hd.bin)

HD_HLA_SRC = go.hla $(shell ls *.hla | grep -v "go.hla")
HD_HLA_OBJ = $(HD_HLA_SRC:.hla=.o)

HD_C_SRC = $(shell ls *.c)
HD_C_OBJ = $(HD_C_SRC:.c=.o)

HD_OBJ = $(HD_HLA_OBJ) $(HD_C_OBJ) $(shell ls ../usrlib/*.o)


HLA_HEADERS = $(shell ls *.hhf ../include/*.hhf ../usrlib/*.hhf)
C_HEADERS = $(shell ls *.h ../include/*.h ../usrlib/*.h)

all: hd.bin

hd.bin: $(HD_OBJ) ../usrlib/all-stamp Makefile
	ld -melf_i386 $(HD_OBJ) -N -M > hd.mem -o hd.bin --oformat binary -T ../script
	ndisasm -b 32 hd.bin > hd.ndisasm
	dd if=/dev/null of=hd.bin seek=$$(( ($(HD_SIZE) / 512) + 1 )) count=1

../usrlib/all-stamp:
	cd ../usrlib && make all

%.o: %.hla Makefile $(HLA_HEADERS)
	hla -c $<
	objdump --disassemble-all -S $@ > $*.objdump

%.o: %.c Makefile $(C_HEADERS)
	cc -fno-builtin -m32 -Wall -fno-stack-protector -fno-leading-underscore -c $<
	objdump --disassemble-all -S $*.o > $*.objdump


clean:
	-rm *.bin
	-rm *.o
	-rm *.objdump
	-rm *.ndisasm
	-rm *.mem
	-rm *~
