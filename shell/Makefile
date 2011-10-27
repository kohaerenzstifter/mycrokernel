SHELL_SIZE = $$(stat -c %s shell.bin)

SHELL_HLA_SRC = go.hla $(shell ls *.hla | grep -v "go.hla")
SHELL_HLA_OBJ = $(SHELL_HLA_SRC:.hla=.o)

SHELL_C_SRC = $(shell ls *.c)
SHELL_C_OBJ = $(SHELL_C_SRC:.c=.o)

SHELL_OBJ = $(SHELL_HLA_OBJ) $(SHELL_C_OBJ) $(shell ls ../usrlib/*.o)


HLA_HEADERS = $(shell ls *.hhf ../include/*.hhf ../usrlib/*.hhf)
C_HEADERS = $(shell ls *.h ../include/*.h ../usrlib/*.h)

all: shell.bin

shell.bin: $(SHELL_OBJ) ../usrlib/all-stamp Makefile
	ld -melf_i386 $(SHELL_OBJ) -N -M > shell.mem -o shell.bin --oformat binary -T ../script
	ndisasm -b 32 shell.bin > shell.ndisasm
	dd if=/dev/null of=shell.bin seek=$$(( ($(SHELL_SIZE) / 512) + 1 )) count=1

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
