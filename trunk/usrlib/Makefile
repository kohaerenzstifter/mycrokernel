HLA_HEADERS = $(shell ls *.hhf ../include/*.hhf ../usrlib/*.hhf)
C_HEADERS = $(shell ls *.h ../include/*.h ../usrlib/*.h)

URSLIB_HLA_SRC = $(shell ls *.hla)
URSLIB_HLA_OBJ = $(URSLIB_HLA_SRC:.hla=.o)

URSLIB_C_SRC = $(shell ls *.c)
URSLIB_C_OBJ = $(URSLIB_C_SRC:.c=.o)

all: all-stamp

all-stamp: $(URSLIB_HLA_OBJ) $(URSLIB_C_OBJ)
	touch all-stamp

%.o: %.hla Makefile $(HLA_HEADERS)
	hla -c $<
	objdump --disassemble-all -S $@ > $*.objdump

%.o: %.c Makefile $(C_HEADERS)
	cc -fno-builtin -m32 -Wall -fno-stack-protector -fno-leading-underscore -c $<
	objdump --disassemble-all -S $@ > $*.objdump
clean:
	-rm *.o *~ all-stamp
