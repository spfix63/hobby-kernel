
%ifndef GDT_INC
%define GDT_INC
 
;bits	16
;*******************************************
; InstallGDT()
;	- Install our GDT
;*******************************************
InstallGDT:
	pusha				; save registers
	push	TSS_data	; writeTss takes 1 argument
	extern	writeTss	; will setup tss struct in gdt
	call	writeTss
	add		esp, 4		; push	TSS_data
	lgdt 	[toc]			; load GDT into GDTR
	mov 	ax, 0x2B      ; Load the index of our TSS structure - The index is
                     ; 0x28, as it is the 5th selector and each is 8 bytes
                     ; long, but we set the bottom two bits (making 0x2B)
                     ; so that it has an RPL of 3, not zero.
	ltr 	ax  
	popa				; restore registers
	ret				; All done!
 
;*******************************************
; Global Descriptor Table (GDT)
;*******************************************
 
gdt_data: 
	dd 0 				; null descriptor
	dd 0 
 
; gdt code:				; code descriptor
	dw 0FFFFh 			; limit low
	dw 0 				; base low
	db 0 				; base middle
	db 10011010b 			; access
	db 11001111b 			; granularity
	db 0 				; base high
 
; gdt data:				; data descriptor
	dw 0FFFFh 			; limit low (Same as code)
	dw 0 				; base low
	db 0 				; base middle
	db 10010010b 			; access
	db 11001111b 			; granularity
	db 0				; base high
 
; user code:				; code descriptor
	dw 0FFFFh 			; limit low
	dw 0 				; base low
	db 0 				; base middle
	db 11111010b 			; access
	db 11011111b 			; granularity
	db 0 				; base high
 
; user data:				; data descriptor
	dw 0FFFFh 			; limit low (Same as code)
	dw 0 				; base low
	db 0 				; base middle
	db 11110010b 			; access
	db 11011111b 			; granularity
	db 0				; base high
	
;set from C
TSS_data:				; data descriptor
	dw 0 			; limit low (Same as code)
	dw 0 				; base low
	db 0 				; base middle
	db 0 			; access
	db 0 			; granularity
	db 0				; base high
	
end_of_gdt:
toc: 
	dw end_of_gdt - gdt_data - 1 	; limit (Size of GDT)
	dd gdt_data 			; base of GDT
 
; give the descriptor offsets names

%define NULL_DESC 0
%define CODE_DESC 0x8
%define DATA_DESC 0x10
 
%endif ;GDT_INC