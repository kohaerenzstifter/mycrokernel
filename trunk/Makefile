GDT_SIZE = $$(stat -c %s kernel/gdt.bin)
_GDT_SIZE = $$(if [ `echo $(GDT_SIZE) % $(SECTOR_SIZE) | bc` -eq 0 ]; then echo $(GDT_SIZE); else echo "(($(GDT_SIZE) / $(SECTOR_SIZE)) + 1) * $(SECTOR_SIZE)" | bc; fi)
SECTOR_SIZE = 512
OFFSET = 1
#OFFSET is 1 because the first 512 bytes to boot from are ONE sector big
KERNEL_SIZE = $$(stat -c %s kernel/kernel.bin)
_KERNEL_SIZE = $$(if [ `echo $(KERNEL_SIZE) % $(SECTOR_SIZE) | bc` -eq 0 ]; then echo $(KERNEL_SIZE); else echo "(($(KERNEL_SIZE) / $(SECTOR_SIZE)) + 1) * $(SECTOR_SIZE)" | bc; fi)
TTY_SIZE = $$(stat -c %s tty/tty.bin)
_TTY_SIZE = $$(if [ `echo $(TTY_SIZE) % $(SECTOR_SIZE) | bc` -eq 0 ]; then echo $(TTY_SIZE); else echo "(($(TTY_SIZE) / $(SECTOR_SIZE)) + 1) * $(SECTOR_SIZE)" | bc; fi)
SHELL_SIZE = $$(stat -c %s shell/shell.bin)
_SHELL_SIZE = $$(if [ `echo $(SHELL_SIZE) % $(SECTOR_SIZE) | bc` -eq 0 ]; then echo $(SHELL_SIZE); else echo "(($(SHELL_SIZE) / $(SECTOR_SIZE)) + 1) * $(SECTOR_SIZE)" | bc; fi)
HD_SIZE = $$(stat -c %s hd/hd.bin)
_HD_SIZE = $$(if [ `echo $(HD_SIZE) % $(SECTOR_SIZE) | bc` -eq 0 ]; then echo $(HD_SIZE); else echo "(($(HD_SIZE) / $(SECTOR_SIZE)) + 1) * $(SECTOR_SIZE)" | bc; fi)

bochs: image
	bochs -f bochs.conf

image: boot/512.bin kernel/gdt.bin kernel/kernel.bin tty/tty.bin shell/shell.bin hd/hd.bin Makefile
	-rm image
	touch image
	dd if=boot/512.bin of=image count=1 seek=0
	dd if=kernel/gdt.bin of=image count=$$(( ($(_GDT_SIZE) / $(SECTOR_SIZE)) )) seek=$(OFFSET)
	dd if=kernel/kernel.bin of=image count=$$(( ($(_KERNEL_SIZE) / $(SECTOR_SIZE)) )) seek=$$(( $(OFFSET) + $(_GDT_SIZE) / $(SECTOR_SIZE)  ))
	dd if=tty/tty.bin of=image count=$$(( ($(_TTY_SIZE) / $(SECTOR_SIZE)) )) seek=$$(( $(OFFSET) + ( $(_GDT_SIZE) + $(_KERNEL_SIZE) ) / $(SECTOR_SIZE)  ))
	dd if=shell/shell.bin of=image count=$$(( ($(_SHELL_SIZE) / $(SECTOR_SIZE)) )) seek=$$(( $(OFFSET) + ( $(_GDT_SIZE) + $(_KERNEL_SIZE) + $(_TTY_SIZE)) / $(SECTOR_SIZE)  ))
	dd if=hd/hd.bin of=image count=$$(( ($(_HD_SIZE) / $(SECTOR_SIZE)) )) seek=$$(( $(OFFSET) + ( $(_GDT_SIZE) + $(_KERNEL_SIZE) + $(_TTY_SIZE) + $(_SHELL_SIZE)) / $(SECTOR_SIZE)  ))
	dd if=/dev/null of=bootdisk.bin count=1 seek=2880
boot/512.bin:
	cd boot && make 512.bin
kernel/gdt.bin:
	cd kernel && make gdt.bin
kernel/kernel.bin:
	cd kernel && make kernel.bin
tty/tty.bin:
	cd tty && make tty.bin
shell/shell.bin:
	cd shell && make shell.bin
hd/hd.bin:
	cd hd && make hd.bin
clean:
	cd boot && make clean && cd ../kernel && make clean && cd ../tty && make clean && cd ../shell && make clean && cd ../hd && make clean && cd ../usrlib && make clean
