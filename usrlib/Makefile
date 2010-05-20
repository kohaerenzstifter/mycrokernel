


syscall.o: syscall.hla
	hla -c syscall.hla
	objdump --disassemble-all -S syscall.o > syscall.objdump
clean:
	-rm *.o *~
