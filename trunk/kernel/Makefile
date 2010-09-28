SECTOR_SIZE = 512
KERNEL_BASE = 83968
KERNEL_SIZE = $$(stat -c %s kernel.bin)
_KERNEL_SIZE = $$(if [ `echo $(KERNEL_SIZE) % $(SECTOR_SIZE) | bc` -eq 0 ]; then echo $(KERNEL_SIZE); else echo "(($(KERNEL_SIZE) / $(SECTOR_SIZE)) + 1) * $(SECTOR_SIZE)" | bc; fi)
VIDEO_SIZE = $$(stat -c %s ../tty/tty.bin)
_VIDEO_SIZE = $$(if [ `echo $(VIDEO_SIZE) % $(SECTOR_SIZE) | bc` -eq 0 ]; then echo $(VIDEO_SIZE); else echo "(($(VIDEO_SIZE) / $(SECTOR_SIZE)) + 1) * $(SECTOR_SIZE)" | bc; fi)
SHELL_SIZE = $$(stat -c %s ../shell/shell.bin)
_SHELL_SIZE = $$(if [ `echo $(SHELL_SIZE) % $(SECTOR_SIZE) | bc` -eq 0 ]; then echo $(SHELL_SIZE); else echo "(($(SHELL_SIZE) / $(SECTOR_SIZE)) + 1) * $(SECTOR_SIZE)" | bc; fi)
HD_SIZE = $$(stat -c %s ../hd/hd.bin)
_HD_SIZE = $$(if [ `echo $(HD_SIZE) % $(SECTOR_SIZE) | bc` -eq 0 ]; then echo $(HD_SIZE); else echo "(($(HD_SIZE) / $(SECTOR_SIZE)) + 1) * $(HD_SIZE)" | bc; fi)

ENVIRONMENT = _HD_SIZE=$(_HD_SIZE) _SHELL_SIZE=$(_SHELL_SIZE) _VIDEO_SIZE=$(_VIDEO_SIZE) SECTOR_SIZE=$(SECTOR_SIZE) KERNEL_BASE=$(KERNEL_BASE)
GDT_ENVIRONMENT = _KERNEL_SIZE=$(_KERNEL_SIZE)

KERNEL_HLA_SRC = go.hla main.hla $(shell ls *.hla | grep -v "gdt.hla" | grep -v "go.hla" | grep -v "main.hla")
KERNEL_HLA_OBJ = $(KERNEL_HLA_SRC:.hla=.o)

KERNEL_C_SRC = $(shell ls *.c)
KERNEL_C_OBJ = $(KERNEL_C_SRC:.c=.o)

KERNEL_OBJ = $(KERNEL_HLA_OBJ) $(KERNEL_C_OBJ) $(shell ls ../usrlib/*.o)

HLA_HEADERS = $(shell ls *.hhf ../include/*.hhf ../usrlib/*.hhf)
C_HEADERS = $(shell ls *.h ../include/*.h ../usrlib/*.h)

all: kernel.bin

%.o: %.hla ../tty/tty.bin ../shell/shell.bin ../hd/hd.bin Makefile $(HLA_HEADERS)
	$(ENVIRONMENT) hla -c $<
	objdump --disassemble-all -S $@ > $*.objdump

%.o: %.c Makefile $(C_HEADERS)
	cc -m32 -Wall -fno-leading-underscore -c $<
	objdump --disassemble-all -S $@ > $*.objdump

kernel.bin: ../usrlib/all-stamp $(KERNEL_OBJ) Makefile
	ld -melf_i386 $(KERNEL_OBJ) -N -M > kernel.mem -o kernel.bin --oformat binary -T script
	ndisasm -b 32 kernel.bin > kernel.ndisasm

gdt.bin: gdt.o Makefile
	ld -melf_i386 gdt.o -N -M > gdt.mem -o gdt.bin --oformat binary -T ../script
	ndisasm -b 32 gdt.bin > gdt.ndisasm

../tty/tty.bin:
	cd ../tty && make tty.bin

../shell/shell.bin:
	cd ../shell && make shell.bin

../hd/hd.bin:
	cd ../hd && make hd.bin

../usrlib/all-stamp:
	cd ../usrlib && make all-stamp

gdt.o: gdt.hla kernel.bin Makefile $(HLA_HEADERS)
	$(GDT_ENVIRONMENT) hla -c gdt.hla
	objdump --disassemble-all -S gdt.o > gdt.objdump


clean:
	-rm *.bin
	-rm *.o
	-rm *.objdump
	-rm *.ndisasm
	-rm *.mem
	-rm *~
	-cd ../tty && make clean
