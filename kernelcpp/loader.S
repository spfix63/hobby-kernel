global loader                           ; making entry point visible to linker
global magic                            ; we will use this in kmain
global mbd                              ; we will use this in kmain

extern kmain                            ; kmain is defined in kmain.cpp
extern start_ctors                      ; beginning and end
extern end_ctors                        ; of the respective
extern start_dtors                      ; ctors and dtors section,
extern end_dtors                        ; declared by the linker script
 
; setting up the Multiboot header - see GRUB docs for details
MODULEALIGN equ  1<<0                   ; align loaded modules on page boundaries
MEMINFO     equ  1<<1                   ; provide memory map
FLAGS       equ  MODULEALIGN | MEMINFO  ; this is the Multiboot 'flag' field
MAGIC       equ    0x1BADB002           ; 'magic number' lets bootloader find the header
CHECKSUM    equ -(MAGIC + FLAGS)        ; checksum required


section .text

align 4
    dd MAGIC
    dd FLAGS
    dd CHECKSUM
 
; reserve initial kernel stack space
STACKSIZE equ 0x4000                    ; that's 16k.

%include "GDT.inc" 

loader:
	mov	[magic], eax                   ; Multiboot magic number
	mov	[mbd], ebx                     ; Multiboot info structure
	
	cli
	
	call	InstallGDT
	
	mov	eax, cr0
	or	al, 01h
	mov	cr0, eax
jmp	CODE_DESC:Stage3

Stage3:	
	mov	ax, DATA_DESC		; set data segments to data selector (0x10)
	mov	ds, ax
	mov	ss, ax
	mov	es, ax
	mov esp, stack + STACKSIZE         ; set up the stack
	
	mov  	ebx, start_ctors               ; call the constructors
	jmp	.ctors_until_end
.call_constructor:
	call 	[ebx]
	add  	ebx, 4
.ctors_until_end:
	cmp	ebx, end_ctors
	jb	.call_constructor
	
	call	kmain		; call kernel proper
	
	mov	ebx, end_dtors                 ; call the destructors
	jmp	.dtors_until_end
.call_destructor:
	sub	ebx, 4
	call	[ebx]
.dtors_until_end:
	cmp	ebx, start_dtors
	ja	.call_destructor
	cli
.hang:
	hlt	; halt machine should kernel return
	jmp	.hang

section .bss
align 4
magic: resd 1
mbd:   resd 1
stack: resb STACKSIZE                   ; reserve 16k stack on a doubleword boundary