.file "irq_wrappers.S"

.intel_syntax noprefix
.text

// The address of the table of handlers (defined in irq.c) 
.extern x86_irq_handler_array

// The address of the table of wrappers (defined below, and shared with irq.c 
.globl x86_irq_wrapper_array

// These pre-handlers are for IRQ (Master PIC)
.irp id, 0,1,2,3,4,5,6,7

	.p2align 2, 0x90

	x86_irq_wrapper_\id:
	.type x86_irq_wrapper_\id,@function

		/*
		 * Backup the CPU context
		 */

		// Fake error code
		push 0

		// Backup the actual context
		push ebp
		mov ebp, esp

		push edi
		push esi
		push edx
		push ecx
		push ebx
		push eax
		sub  esp, 2
		pushw ss
		pushw ds
		pushw es
		pushw fs
		pushw gs

		// Send EOI to PIC. See Intel 8259 datasheet
		mov al, 0x20
		outb 0x20, al

		/*
		 * Call the handler with IRQ number as argument
		 */
		push \id
		lea  edi, x86_irq_handler_array
		call [edi+4*\id]
		add  esp, 4

		popw  gs
		popw  fs
		popw  es
		popw  ds
		popw  ss
		add esp, 2
		pop eax
		pop ebx
		pop ecx
		pop edx
		pop esi
		pop edi
		pop ebp

		// Remove fake error code
		add esp, 4

		iret
	.endr


// These pre-handlers are for IRQ (Slave PIC)
.irp id, 8,9,10,11,12,13,14,15

	.p2align 2, 0x90

	x86_irq_wrapper_\id:
	.type x86_irq_wrapper_\id,@function

		/*
		 * Backup the CPU context
		 */

		// Fake error code
		push 0

		// Backup the actual context
		push ebp
		mov  ebp, esp

		// Restore the context
		push edi
		push esi
		push edx
		push ecx
		push ebx
		push eax
		sub  esp, 2
		pushw ss
		pushw ds
		pushw es
		pushw fs
		pushw gs

		// Send EOI to PIC. See Intel 8259 datasheet	
		movb  al, 0x20
		outb  0xa0, al
		outb  0x20, al

		/*
		 * Call the handler with IRQ number as argument
		 */
		push \id
		lea  edi, x86_irq_handler_array
		call [edi+4*\id]
		add  esp, 4

		// Restore the context
		popw gs
		popw fs
		popw es
		popw ds
		popw ss
		add  esp, 2
		pop  eax
		pop  ebx
		pop  ecx
		pop  edx
		pop  esi
		pop  edi
		pop  ebp

		// Remove fake error code
		add  esp, 4

		iret
	.endr


.section ".rodata"
msg_nested_level_overflow:
	.string "irq_wrappers.S: IRQ Nested level overflow ! System halted."


// Build the x86_irq_wrapper_array, shared with irq.c
.p2align 5, 0x0
x86_irq_wrapper_array:
	.irp id, 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15
		.long (x86_irq_wrapper_\id)
	.endr
