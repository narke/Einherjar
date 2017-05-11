; @see http://wiki.osdev.org/Bare_Bones
; Slightly modified by me

extern roentgenium_main	; this is our kernel's entry point

; Setting up the Multiboot header - see GRUB docs for details
MBALIGN		equ	1<<0		; align loaded modules on page boundaries
MEMINFO		equ	1<<1		; provide memory map
FLAGS		equ	MBALIGN | MEMINFO	; this is the Multiboot 'flag' field
MAGIC		equ	0x1BADB002		; 'magic number' lets bootloader find the header
CHECKSUM	equ	-(MAGIC + FLAGS)	; checksum required to prove that we are multiboot
STACK_SIZE	equ	0x4000		; our stack size is 16KiB


; The multiboot header must come first.
section .multiboot

; Multiboot header must be aligned on a 4-byte boundary
align 4

multiboot_header:
dd MAGIC
dd FLAGS
dd -(MAGIC + FLAGS)

; The beginning of our kernel code
section .text

global multiboot_entry
multiboot_entry:
	mov esp, stack + STACK_SIZE	; set up the stack
	mov [magic], ebx		; multiboot magic number
	mov [multiboot_info], eax	; multiboot data structure

	call roentgenium_main		; calling the kernel

hang:
	hlt				; something bad happened, machine halted
	jmp hang


section .bss nobits align=4
; Reserve initial kernel stack space
stack:          resb STACK_SIZE ; reserve 16 KiB stack
multiboot_info: resd 1          ; we will use this in kernel's main
magic:          resd 1          ; we will use this in kernel's main
