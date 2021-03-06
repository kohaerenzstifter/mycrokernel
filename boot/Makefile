KERNEL_BASE = 83968
SECTOR_SIZE = 512
STACK_SIZE = 8192

#OFFSET is 1 because the first 512 bytes to boot from are ONE sector big
OFFSET = 1
KERNEL_SIZE = $$(stat -c %s ../kernel/kernel.bin)
__KERNEL_SIZE = $$(if [ `echo $(KERNEL_SIZE) % $(SECTOR_SIZE) | bc` -eq 0 ]; then echo $(KERNEL_SIZE); else echo "(($(KERNEL_SIZE) / $(SECTOR_SIZE)) + 1) * $(SECTOR_SIZE)" | bc; fi)
_KERNEL_SIZE = $$(echo $(__KERNEL_SIZE) + $(STACK_SIZE))
GDT_SIZE = $$(stat -c %s ../kernel/gdt.bin)
_GDT_SIZE = $$(if [ `echo $(GDT_SIZE) % $(SECTOR_SIZE) | bc` -eq 0 ]; then echo $(GDT_SIZE); else echo "(($(GDT_SIZE) / $(SECTOR_SIZE)) + 1) * $(SECTOR_SIZE)" | bc; fi)
VIDEO_SIZE = $$(stat -c %s ../tty/tty.bin)
__VIDEO_SIZE = $$(if [ `echo $(VIDEO_SIZE) % $(SECTOR_SIZE) | bc` -eq 0 ]; then echo $(VIDEO_SIZE); else echo "(($(VIDEO_SIZE) / $(SECTOR_SIZE)) + 1) * $(SECTOR_SIZE)" | bc; fi)
_VIDEO_SIZE = $$(echo $(__VIDEO_SIZE) + $(STACK_SIZE))
SHELL_SIZE = $$(stat -c %s ../shell/shell.bin)
__SHELL_SIZE = $$(if [ `echo $(SHELL_SIZE) % $(SECTOR_SIZE) | bc` -eq 0 ]; then echo $(SHELL_SIZE); else echo "(($(SHELL_SIZE) / $(SECTOR_SIZE)) + 1) * $(SECTOR_SIZE)" | bc; fi)
_SHELL_SIZE = $$(echo $(__SHELL_SIZE) + $(STACK_SIZE))
HD_SIZE = $$(stat -c %s ../hd/hd.bin)
__HD_SIZE = $$(if [ `echo $(HD_SIZE) % $(SECTOR_SIZE) | bc` -eq 0 ]; then echo $(HD_SIZE); else echo "(($(HD_SIZE) / $(SECTOR_SIZE)) + 1) * $(HD_SIZE)" | bc; fi)
_HD_SIZE = $$(echo $(__HD_SIZE) + $(STACK_SIZE))

NO_SEGMENTS = $$(( $(GDT_SIZE) / 8 ))

ENVIRONMENT = KERNEL_BASE=$(KERNEL_BASE) SECTOR_SIZE=$(SECTOR_SIZE) OFFSET=$(OFFSET) _KERNEL_SIZE=$(_KERNEL_SIZE) _GDT_SIZE=$(_GDT_SIZE) _VIDEO_SIZE=$(_VIDEO_SIZE) _SHELL_SIZE=$(_SHELL_SIZE) _HD_SIZE=$(_HD_SIZE) NO_SEGMENTS=$(NO_SEGMENTS)

512.bin: 512.asm ../kernel/kernel.bin ../kernel/gdt.bin ../tty/tty.bin ../shell/shell.bin Makefile
	$(ENVIRONMENT) nasm -f bin 512.asm -o 512.bin
../kernel/kernel.bin:
	cd ../kernel && make kernel.bin
../kernel/gdt.bin:
	cd ../kernel && make gdt.bin
../tty/tty.bin:
	cd ../tty && make tty.bin
../shell/shell.bin:
	cd ../shell && make shell.bin

clean:
	-rm *.bin
	-rm *.o
	-rm *.objdump
	-rm *.ndisasm
	-rm *.mem
	-rm *~
	-cd ../kernel && make clean && cd ../tty && make clean
