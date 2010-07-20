KERNEL_BASE = 83968
SECTOR_SIZE = 512
#OFFSET is 1 because the first 512 bytes to boot from are ONE sector big
OFFSET = 1
KERNEL_SIZE = $$(stat -c %s ../kernel/kernel.bin)
_KERNEL_SIZE = $$(if [ `echo $(KERNEL_SIZE) % $(SECTOR_SIZE) | bc` -eq 0 ]; then echo $(KERNEL_SIZE); else echo "(($(KERNEL_SIZE) / $(SECTOR_SIZE)) + 1) * $(SECTOR_SIZE)" | bc; fi)
GDT_SIZE = $$(stat -c %s ../kernel/gdt.bin)
_GDT_SIZE = $$(if [ `echo $(GDT_SIZE) % $(SECTOR_SIZE) | bc` -eq 0 ]; then echo $(GDT_SIZE); else echo "(($(GDT_SIZE) / $(SECTOR_SIZE)) + 1) * $(SECTOR_SIZE)" | bc; fi)
VIDEO_SIZE = $$(stat -c %s ../video/video.bin)
_VIDEO_SIZE = $$(if [ `echo $(VIDEO_SIZE) % $(SECTOR_SIZE) | bc` -eq 0 ]; then echo $(VIDEO_SIZE); else echo "(($(VIDEO_SIZE) / $(SECTOR_SIZE)) + 1) * $(SECTOR_SIZE)" | bc; fi)
SHELL_SIZE = $$(stat -c %s ../shell/shell.bin)
_SHELL_SIZE = $$(if [ `echo $(SHELL_SIZE) % $(SECTOR_SIZE) | bc` -eq 0 ]; then echo $(SHELL_SIZE); else echo "(($(SHELL_SIZE) / $(SECTOR_SIZE)) + 1) * $(SECTOR_SIZE)" | bc; fi)


NO_SEGMENTS = $$(( $(GDT_SIZE) / 8 ))

ENVIRONMENT = KERNEL_BASE=$(KERNEL_BASE) SECTOR_SIZE=$(SECTOR_SIZE) OFFSET=$(OFFSET) _KERNEL_SIZE=$(_KERNEL_SIZE) _GDT_SIZE=$(_GDT_SIZE) _VIDEO_SIZE=$(_VIDEO_SIZE) _SHELL_SIZE=$(_SHELL_SIZE) NO_SEGMENTS=$(NO_SEGMENTS)

512.bin: 512.asm ../kernel/kernel.bin ../kernel/gdt.bin ../video/video.bin Makefile
	$(ENVIRONMENT) nasm -f bin 512.asm -o $(SECTOR_SIZE).bin
../kernel/kernel.bin:
	cd ../kernel && make kernel.bin
../kernel/gdt.bin:
	cd ../kernel && make gdt.bin
../video/video.bin:
	cd ../video && make video.bin

clean:
	-rm *.bin
	-rm *.o
	-rm *.objdump
	-rm *.ndisasm
	-rm *.mem
	-rm *~
	-cd ../kernel && make clean && cd ../video && make clean
