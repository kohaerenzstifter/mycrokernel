SECTOR_SIZE = 512
#KERNEL_BASE = 83712
KERNEL_BASE = 83968
KERNEL_SIZE = $$(stat -c %s kernel.bin)
_KERNEL_SIZE = $$(if [ `echo $(KERNEL_SIZE) % $(SECTOR_SIZE) | bc` -eq 0 ]; then echo $(KERNEL_SIZE); else echo "(($(KERNEL_SIZE) / $(SECTOR_SIZE)) + 1) * $(SECTOR_SIZE)" | bc; fi)
VIDEO_SIZE = $$(stat -c %s ../video/video.bin)

gdt.bin: gdt.o Makefile
	ld -melf_i386 gdt.o -N -M > gdt.mem -o gdt.bin --oformat binary -T \
	../script
	ndisasm -b 32 gdt.bin > gdt.ndisasm

kernel.bin: go.o table.o main.o video.o utilities_HLA.o utilities_C.o ../usrlib/syscall.o \
	lowlevel.o timer.o syscall.o keyboard.o Makefile
	ld -melf_i386 go.o main.o video.o ../usrlib/syscall.o utilities_HLA.o \
	utilities_C.o keyboard.o \
	lowlevel.o timer.o syscall.o table.o \
  -N -M > kernel.mem -o kernel.bin --oformat \
  binary -T ../script
	  ndisasm -b 32 kernel.bin > kernel.ndisasm
../video/video.bin:
	cd ../video && make video.bin
go.o: go.hla always.hhf prototype.hhf
	hla -c go.hla
	objdump --disassemble-all -S go.o > go.objdump
main.o:	main.hla Makefile always.hhf types.hhf const.hhf prototype.hhf external.hhf ../video/video.bin
	VIDEO_SIZE=$(VIDEO_SIZE) SECTOR_SIZE=$(SECTOR_SIZE) KERNEL_BASE=$(KERNEL_BASE) hla -c main.hla
	objdump --disassemble-all -S main.o > main.objdump
utilities_C.o:	utilities_C.c Makefile
	cc -Wall -fno-leading-underscore -c utilities_C.c
	objdump --disassemble-all -S utilities_C.o > utilities_C.objdump
utilities_HLA.o: utilities_HLA.hla Makefile
	hla -c utilities_HLA.hla
	objdump --disassemble-all -S utilities_HLA.o > utilities_HLA.objdump
video.o: video.hla Makefile always.hhf types.hhf const.hhf prototype.hhf external.hhf ../include/character.hhf ../include/always.hhf
	hla -c video.hla
	objdump --disassemble-all -S video.o > video.objdump
keyboard.o: keyboard.hla const.hhf prototype.hhf types.hhf external.hhf \
  ../include/character.hhf Makefile ../include/always.hhf
	hla -c keyboard.hla
timer.o: timer.hla const.hhf prototype.hhf external.hhf ../include/always.hhf
	hla -c timer.hla
	objdump --disassemble-all -S timer.o > timer.objdump
table.o: table.hla types.hhf const.hhf prototype.hhf external.hhf
	hla -c table.hla
	objdump --disassemble-all -S table.o > table.objdump
gdt.o: gdt.hla types.hhf kernel.bin
	_KERNEL_SIZE=$(_KERNEL_SIZE) hla -c gdt.hla
	objdump --disassemble-all -S gdt.o > gdt.objdump
syscall.o: syscall.hla prototype.hhf
	hla -c syscall.hla
	objdump --disassemble-all -S syscall.o > syscall.objdump
lowlevel.o: lowlevel.hla always.hhf types.hhf const.hhf prototype.hhf \
  external.hhf ../include/always.hhf Makefile
	hla -c lowlevel.hla
	objdump --disassemble-all -S lowlevel.o > lowlevel.objdump
clean:
	-rm *.bin
	-rm *.o
	-rm *.objdump
	-rm *.ndisasm
	-rm *.mem
	-rm *~
	-cd ../video && make clean
