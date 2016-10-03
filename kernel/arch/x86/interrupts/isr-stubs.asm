
; @file isr_stubs.asm
; @author Konstantin Tcholokachvili
; @see http://www.osdever.net/bkerndev/Docs/isrs.htm
; @date 2013, 2016
;
; Interrupt Service Routines (ISRs)

%macro ISR_NO_ERROR_CODE 1
	[global isr%1]
	isr%1:
		cli
		push byte 0
		push byte %1
		jmp isr_common_stub
%endmacro

%macro ISR_ERROR_CODE 1
	[global isr%1]
	isr%1:
		cli
		push byte %1
		jmp isr_common_stub
%endmacro


; We call a C function in here. We need to let the assembler know
; that 'x86_isr_handler' exists in another file

[extern x86_isr_handler]


; This is our common ISR stub. It saves the processor state, sets
; up for kernel mode segments, calls the C-level isr handler,
; and finally restores the stack frame.

isr_common_stub:
    pusha
    push ds
    push es
    push fs
    push gs
    mov ax, 0x10	; Load the Kernel Data Segment descriptor!
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov eax, esp	; Push us the stack
    push eax
    mov eax, x86_isr_handler
    call eax		; A special call, preserves the 'eip' register
    pop eax
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8		; Cleans up the pushed error code and pushed ISR number
    iret		; pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP!


ISR_NO_ERROR_CODE 0  ; Divide By Zero Exception
ISR_NO_ERROR_CODE 1  ; Debug Exception
ISR_NO_ERROR_CODE 2  ; Non Maskable Interrupt Exception
ISR_NO_ERROR_CODE 3  ; Int 3 Exception
ISR_NO_ERROR_CODE 4  ; INTO Exception
ISR_NO_ERROR_CODE 5  ; Out of Bounds Exception
ISR_NO_ERROR_CODE 6  ; Invalid Opcode Exception
ISR_NO_ERROR_CODE 7  ; Coprocessor Not Available Exception
ISR_ERROR_CODE 8     ; Double Fault Exception (With Error Code!)
ISR_NO_ERROR_CODE 9  ; Coprocessor Segment Overrun Exception
ISR_ERROR_CODE 10    ; Bad TSS Exception (With Error Code!)
ISR_ERROR_CODE 11    ; Segment Not Present Exception (With Error Code!)
ISR_ERROR_CODE 12    ; Stack Fault Exception (With Error Code!)
ISR_ERROR_CODE 13    ; General Protection Fault Exception (With Error Code!)
ISR_ERROR_CODE 14    ; Page Fault Exception (With Error Code!)
ISR_NO_ERROR_CODE 15 ; Reserved Exception
ISR_NO_ERROR_CODE 16 ; Floating Point Exception
ISR_NO_ERROR_CODE 17 ; Alignment Check Exception
ISR_NO_ERROR_CODE 18 ; Machine Check Exception
ISR_NO_ERROR_CODE 19 ; Reserved
ISR_NO_ERROR_CODE 20 ; Reserved
ISR_NO_ERROR_CODE 21 ; Reserved
ISR_NO_ERROR_CODE 22 ; Reserved
ISR_NO_ERROR_CODE 23 ; Reserved
ISR_NO_ERROR_CODE 24 ; Reserved
ISR_NO_ERROR_CODE 25 ; Reserved
ISR_NO_ERROR_CODE 26 ; Reserved
ISR_NO_ERROR_CODE 27 ; Reserved
ISR_NO_ERROR_CODE 28 ; Reserved
ISR_NO_ERROR_CODE 29 ; Reserved
ISR_NO_ERROR_CODE 30 ; Reserved
ISR_NO_ERROR_CODE 31 ; Reserved
