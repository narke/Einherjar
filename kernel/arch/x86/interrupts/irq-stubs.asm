; @file irq_stubs.asm
; @author Konstantin Tcholokachvili
; @date 2014, 2016
;
; Interrupt Requests (IRQs)

section .text

; The address of the table of handlers (defined in irq.c)
[extern x86_irq_handler_array]

; The address of the table of wrappers (defined below, and shared with irq.c)
[global x86_irq_wrapper_array]

%macro SAVE_REGISTERS 0
	push edi
	push esi
	push edx
	push ecx
	push ebx
	push eax
	sub  esp, 2
	push word ss
	push word ds
	push word es
	push word fs
	push word gs
%endmacro

%macro RESTORE_REGISTERS 0
	pop word  gs
	pop word  fs
	pop word  es
	pop word  ds
	pop word  ss
	add esp, 2
	pop eax
	pop ebx
	pop ecx
	pop edx
	pop esi
	pop edi
	pop ebp
%endmacro

; These pre-handlers are for IRQ (Master PIC)
%macro X86_IRQ_WRAPPER_MASTER 1
	align 4

	x86_irq_wrapper_%1:
		; Fake error code
		push 0

		; Backup the actual context
		push ebp
		mov ebp, esp

		SAVE_REGISTERS

		; Send EOI to PIC. See Intel 8259 datasheet
		mov al, 0x20
		out byte 0x20, al

		; Call the handler with IRQ number as argument
		push %1
		lea  edi, [x86_irq_handler_array]
		call [edi+4*%1]
		add  esp, 4

		RESTORE_REGISTERS

		; Remove fake error code
		add esp, 4

		iret
%endmacro

; These pre-handlers are for IRQ (Slave PIC)
%macro X86_IRQ_WRAPPER_SLAVE 1
	align 4

	x86_irq_wrapper_%1:
		; Fake error code
		push 0

		; Backup the actual context
		push ebp
		mov ebp, esp

		SAVE_REGISTERS

		; Send EOI to PIC. See Intel 8259 datasheet
		mov byte  al, 0x20
		out byte 0xa0, al
		out byte 0x20, al

		; Call the handler with IRQ number as argument
		push %1
		lea  edi, [x86_irq_handler_array]
		call [edi+4*%1]
		add  esp, 4

		RESTORE_REGISTERS

		; Remove fake error code
		add  esp, 4

		iret
%endmacro

X86_IRQ_WRAPPER_MASTER 0
X86_IRQ_WRAPPER_MASTER 1
X86_IRQ_WRAPPER_MASTER 2
X86_IRQ_WRAPPER_MASTER 3
X86_IRQ_WRAPPER_MASTER 4
X86_IRQ_WRAPPER_MASTER 5
X86_IRQ_WRAPPER_MASTER 6
X86_IRQ_WRAPPER_MASTER 7
X86_IRQ_WRAPPER_SLAVE  8
X86_IRQ_WRAPPER_SLAVE  9
X86_IRQ_WRAPPER_SLAVE  10
X86_IRQ_WRAPPER_SLAVE  11
X86_IRQ_WRAPPER_SLAVE  12
X86_IRQ_WRAPPER_SLAVE  13
X86_IRQ_WRAPPER_SLAVE  14
X86_IRQ_WRAPPER_SLAVE  15

section .rodata

; Build the x86_irq_wrapper_array, shared with irq.c
align 32, db 0
x86_irq_wrapper_array:
	dd x86_irq_wrapper_0
	dd x86_irq_wrapper_1
	dd x86_irq_wrapper_2
	dd x86_irq_wrapper_3
	dd x86_irq_wrapper_4
	dd x86_irq_wrapper_5
	dd x86_irq_wrapper_6
	dd x86_irq_wrapper_7
	dd x86_irq_wrapper_8
	dd x86_irq_wrapper_9
	dd x86_irq_wrapper_10
	dd x86_irq_wrapper_11
	dd x86_irq_wrapper_12
	dd x86_irq_wrapper_13
	dd x86_irq_wrapper_14
	dd x86_irq_wrapper_15

