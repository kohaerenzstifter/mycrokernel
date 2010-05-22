;Layout of 1st MB of Memory:
;0h-03ffh: Real Mode Interrupt Vector Table (RAM)
;0400h-04ffh: BIOS data area (RAM)
;0500h-09ffff: free memory (RAM) (=09FB00h bytes)..see(1)
;0a0000h-0bffffh: video memory (RAM)  1280-
;0c0000-0c7fff: video bios (ROM)
;0c0000-oeffff: BIOS shadow area
;0f0000-0fffff: System bios (ROM)

;How we're going to make use of this first MB:
;512-byte bootsector after relocation: 0500-06ff
;stack: 0700h-106fch
;gdt: 10700-126ff
;(idt: 12700-146ff) not yet written
;boot: 14700-(09ffff)

%define MAXSECTOR 18
%define NO_SECTORS 18
%define MAXHEAD 1
%define NO_HEADS 2
%define _512_BASE 0500h
%define STACK_BASE 0700h
%define STACKTOP 0fffch
%define OFFSET %!OFFSET
%define _GDT_SIZE %!_GDT_SIZE
%define GDT_BASE 010700h
%define _KERNEL_SIZE %!_KERNEL_SIZE
%define _VIDEO_SIZE %!_VIDEO_SIZE
%define KERNEL_BASE %!KERNEL_BASE ;014700h
%define SECTOR_SIZE %!SECTOR_SIZE
%define NO_SEGMENTS %!NO_SEGMENTS ;number of segments in gdt by default

  BITS 16

  jmp 07C0h:start
start:

  mov     al, 0d1h	;enable the
  out     064h, al	;A20
  mov     al, 0dfh	;address
  out     060h, al	;line

  mov [bootdrive], dl	;store the boot drive

  mov cx, 256		;256 words to copy
  mov ax, 0	
  mov ds, ax		;This is the source address from
  mov si, 07c00h		;where we copy the 512 byte boot code
  mov es, ax		;This is the destination address to
  mov di, _512_BASE	;where we copy the 512 byte boot code
  cld			;clear the direction flag
  rep movsw		;copy the boot code!

  jmp _512_BASE/010h:here	;jump to the relocated boot sector!

here:

  mov ax, cs	
  mov ds, ax		;Initialise
  mov es, ax		;the
  mov fs, ax		;data
  mov gs, ax		;registers
  mov ax, STACK_BASE/010h		;and
  cli			;the
  mov ss, ax		;stack
  mov esp, STACKTOP	;of
  sti			;course
  mov si, rlctn_msg	;Now let's tell the user
  call printstring	;that we have successfully relocated



  mov di, memmap				;di:=&memmap
  mov ebx, 0				;ebx:=start of entire memory
  mov edx, 0534D4150h ;in the loop??	;edx:="SMAP"
loop_getmemmap:
  mov ecx, 20				;We want a 20-bytes long descriptor
  mov eax, 0e820h				;Interrupt subroutine (Query System Address Map)
  int 15h					;Call the interrupt!
  inc dword [no_memsegs]			;We've got another memory descriptor
  cmp ebx, 0				;Check if this was the last descriptor there is
  jz end_getmemmap			;and if so, jump to the end of this loop
  add di, 20				;di:=di+@sizeof(memdesc)
  jmp loop_getmemmap

end_getmemmap:

  mov dl, [bootdrive]		;The drive we booted from
  mov ah, 00h ;reset disk drive	;The function to reset drive in dl
  int 13h				;Call interrupt 13h ah
  jc rsterr			;Display error message if drive reset failed
  
  mov ax, GDT_BASE/010h		;Segment where
  mov es, ax			;to load gdt
  mov bx, 0			;Address in segment es where to load gdt
  mov si, _GDT_SIZE/SECTOR_SIZE	;Read so many sectors
  mov cx, OFFSET + 1
;	mov cx, 02h			;Cylinder 0, sector 2
  mov dh, 0			;Head 0
  call loop_load


  mov ax, KERNEL_BASE/010h				;Segment where
  mov es, ax						;to load boot
  mov bx, 0						;Address in segment es where to load boot
  mov si, (_KERNEL_SIZE + _VIDEO_SIZE)/SECTOR_SIZE	;Read so many sectors
  mov cx, _GDT_SIZE/512 + 1 + OFFSET
;	mov cx, 03h			;Cylinder 0, sector 3
  mov dh, 0			;Head 0
  call loop_load


  mov ax, cs			;Reset es back to the
  mov es, ax			;base of this code
  mov si, gdtldd_msg		;tell the user that
  call printstring		;we have loaded the gdt from disk
  cli				;disable interrupts before entering pm!
  lgdt [gdt_desc]			;load gdt register
  mov eax, cr0			;Enable
  or eax, 1			;protected
  mov cr0, eax			;mode
  ;remap interrupts here
  ;reenable interrupts here

  jmp 20h:enter_32		;jump into pm!


align 2
rsterr:
  mov si, rsterr_msg
  call printstring
  cli
  hlt
align 2
lderr:
  mov si, lderr_msg
  call printstring
  cli
  hlt
align 2
loop_load:
  cmp si, 0h
  je end_loop_load
  call loadSector
  dec si
  inc cl
  mov ax, es
  add ax, SECTOR_SIZE/10h
  mov es, ax
  jmp loop_load
end_loop_load:
  ret
align 2
loadSector:
head_increase:
  cmp cl, MAXSECTOR + 1
  jne cylinder_increase
  sub cl, NO_SECTORS
  inc dh
  jmp head_increase
cylinder_increase:
  cmp dh, MAXHEAD + 1
  jne over_cylinder_increase
  sub dh, NO_HEADS
  inc ch
  jmp cylinder_increase
over_cylinder_increase:
  mov ah, 02h		;Function to read from drive
  mov al, 1
  int 13h
  jc lderr
  ret
align 2
;routine printstring: will print zero-terminated string from ds:si.
printstring:
  pusha
  pushf
  mov bx, 7 ;use the right page
  mov ah, 0Eh
nxtprintstring:
  lodsb
  or al, al
  jz endprintstring
  int 10h
  jmp nxtprintstring
endprintstring:
  popf
  popa
  ret
align 2
rlctn_msg:
  db 0ah, 0dh, 'Successfully relocated myself...',0
align 2
rsterr_msg:
  db 0ah, 0dh, 'Reset boot drive error!',0
align 2
lderr_msg:
  db 0ah, 0dh, 'Load Error!',0
align 2
gdtldd_msg:
  db 0ah, 0dh, 'Successfully loaded GDT!',0
align 2
bootdrive:
  db 0
BITS 32

enter_32:
  mov ax, 010h
  mov ds, ax	;ds is the base address where boot is loaded
  mov es, ax
  mov ax, 30h
  mov fs, ax	;fs is the base of addressable memory (offset 0!)
  mov ax, 028h	
  mov gs, ax
  mov ax, 018h
  ;DONT forget to disable interrupts here, if they are enabled!!!
  mov esp, 0fffffffch
  mov ss, ax
  ;(reenable interrupts here)
  push _512_BASE+pass 	;the parameter for start
  push 0			;this is a dummy because HLA procedures
			  ;don't support far calls

  jmp 08h:0h

;this is the data that is passed to the kernel with a pointer

align 4
pass:
gdt_desc:
  dw (8*NO_SEGMENTS)-1	;limit of the gdt
  dd GDT_BASE		;base address of the gdt
align 4
no_memsegs:
  dd 0
align 4
memmap:

times 510-($-$$) db 0 ; Fill the file with 0s
dw 0aa55h
