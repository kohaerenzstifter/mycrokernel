VIDEO_SIZE = $$(stat -c %s video.bin)

video.bin: go.o main.o table.o utilities.o Makefile ../usrlib/syscall.o ../usrlib/syscall.h
	ld -melf_i386 go.o main.o table.o utilities.o ../usrlib/syscall.o -N -M > video.mem -o video.bin --oformat binary -T ../script
	ndisasm -b 32 video.bin > video.ndisasm
	dd if=/dev/null of=video.bin seek=$$(( ($(VIDEO_SIZE) / 512) + 1 )) count=1
main.o: main.c Makefile
	cc -Wall -fno-leading-underscore -c main.c
	objdump --disassemble-all -S main.o > main.objdump
../usrlib/syscall.o:
	cd ../usrlib && make syscall.o
go.o: go.hla prototype.hhf Makefile
	hla -c go.hla
	objdump --disassemble-all -S go.o > go.objdump
table.o: table.hla external.hhf Makefile
	hla -c table.hla
	objdump --disassemble-all -S table.o > table.objdump

utilities.o: utilities.hla external.hhf prototype.hhf ../include/const.hhf ../include/character.hhf ../include/always.hhf Makefile
	hla -c utilities.hla
	objdump --disassemble-all -S utilities.o > utilities.objdump

clean:
	-rm *.bin
	-rm *.o
	-rm *.objdump
	-rm *.ndisasm
	-rm *.mem
	-rm *~
