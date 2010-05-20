GDT_SIZE = $$(stat -c %s kernel/gdt.bin)
_GDT_SIZE = $$(if [ `echo $(GDT_SIZE) % $(SECTOR_SIZE) | bc` -eq 0 ]; then echo $(GDT_SIZE); else echo "(($(GDT_SIZE) / $(SECTOR_SIZE)) + 1) * $(SECTOR_SIZE)" | bc; fi)
SECTOR_SIZE = 512
OFFSET = 1
#OFFSET is 1 because the first 512 bytes to boot from are ONE sector big
KERNEL_SIZE = $$(stat -c %s kernel/kernel.bin)
_KERNEL_SIZE = $$(if [ `echo $(KERNEL_SIZE) % $(SECTOR_SIZE) | bc` -eq 0 ]; then echo $(KERNEL_SIZE); else echo "(($(KERNEL_SIZE) / $(SECTOR_SIZE)) + 1) * $(SECTOR_SIZE)" | bc; fi)
VIDEO_SIZE = $$(stat -c %s video/video.bin)
_VIDEO_SIZE = $$(if [ `echo $(VIDEO_SIZE) % $(SECTOR_SIZE) | bc` -eq 0 ]; then echo $(VIDEO_SIZE); else echo "(($(VIDEO_SIZE) / $(SECTOR_SIZE)) + 1) * $(SECTOR_SIZE)" | bc; fi)

bochs: all
	bochs -f bochs.conf

all: image

image: boot/512.bin kernel/gdt.bin kernel/kernel.bin video/video.bin Makefile
	-rm image
	touch image
	dd if=boot/512.bin of=image count=1 seek=0
	dd if=kernel/gdt.bin of=image count=$$(( ($(_GDT_SIZE) / $(SECTOR_SIZE)) )) seek=$(OFFSET)
	dd if=kernel/kernel.bin of=image count=$$(( ($(_KERNEL_SIZE) / $(SECTOR_SIZE)) )) seek=$$(( $(OFFSET) + $(_GDT_SIZE) / $(SECTOR_SIZE)  ))
	dd if=video/video.bin of=image count=$$(( ($(_VIDEO_SIZE) / $(SECTOR_SIZE)) )) seek=$$(( $(OFFSET) + ( $(_GDT_SIZE) + $(_KERNEL_SIZE) ) / $(SECTOR_SIZE)  ))
	dd if=/dev/null of=bootdisk.bin count=1 seek=2880
boot/512.bin:
	cd boot && make 512.bin
kernel/gdt.bin:
	cd kernel && make gdt.bin
kernel/kernel.bin:
	cd kernel && make kernel.bin
video/video.bin:
	cd video && make video.bin

clean:
	cd boot && make clean && cd ../kernel && make clean && cd ../video && make clean && cd ../usrlib && make clean
