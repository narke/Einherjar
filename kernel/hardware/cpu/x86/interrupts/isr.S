/**
 * @file isr.S
 * @author Konstantin Tcholokachvili
 * Interrupt Service Routines (ISRs)
 * @see http://www.osdever.net/bkerndev/Docs/isrs.htm
 * @date 2013
 */

/* In just a few pages in this tutorial, we will add our Interrupt
	Service Routines (ISRs) right here! */

.global _isr0
.global _isr1
.global _isr2
.global _isr3
.global _isr4
.global _isr5
.global _isr6
.global _isr7
.global _isr8
.global _isr9
.global _isr10
.global _isr11
.global _isr12
.global _isr13
.global _isr14
.global _isr15
.global _isr16
.global _isr17
.global _isr18
.global _isr19
.global _isr20
.global _isr21
.global _isr22
.global _isr23
.global _isr24
.global _isr25
.global _isr26
.global _isr27
.global _isr28
.global _isr29
.global _isr30
.global _isr31

/*  0: Divide By Zero Exception */
_isr0:
    cli
    pushl $0	; A normal ISR stub that pops a dummy error code
				; to keep a uniform stack frame
    pushl $0
    jmp isr_common_stub

/*  1: Debug Exception */
_isr1:
    cli
    pushl $0
    pushl $1
    jmp isr_common_stub
    
;  2: Non Maskable Interrupt Exception
_isr2:
    cli
    pushl $0
    pushl $2
    jmp isr_common_stub

;  3: Breakpoint Exception
_isr3:
    cli
    pushl $0
    pushl $3
    jmp isr_common_stub
    

;  4: Into Detected Overflow Exception
_isr4:
    cli
    pushl $0
    pushl $4
    jmp isr_common_stub
    
;  5: Out of Bounds Exception
_isr5:
    cli
    pushl $0
    pushl $5
    jmp isr_common_stub
    
;  6: Invalid Opcode Exception
_isr6:
    cli
    pushl $0
    pushl $6
    jmp isr_common_stub
    
;  7: No Coprocessor Exception
_isr7:
    cli
    pushl $0
    pushl $6
    jmp isr_common_stub

/*  8: Double Fault Exception (With Error Code!) */
_isr8:
    cli
    pushl $8		/* Note that we DON'T push a value on the stack in this one!
					It pushes one already! Use this type of stub for exceptions
					that pop error codes! */
    jmp isr_common_stub

/*  9: Coprocessor Segment Overrun Exception */
_isr9:
    cli
    pushl $0
    pushl $9
    jmp isr_common_stub

/*  10: Bad TSS Exception (With Error Code!) */
_isr10:
    cli
    pushl $10
    jmp isr_common_stub
    
/*  11: Segment Not Present Exception (With Error Code!) */
_isr11:
    cli
    pushl $11
    jmp isr_common_stub

/*  12: Stack Fault Exception (With Error Code!) */
_isr12:
    cli
    pushl $12
    jmp isr_common_stub

/*  13: General Protection Fault Exception (With Error Code!) */
_isr13:
    cli
    pushl $13
    jmp isr_common_stub

/*  14: Page Fault Exception (With Error Code!) */
_isr14:
    cli
    pushl $14
    jmp isr_common_stub

/*  15: Unknown Interrupt Exception */
_isr15:
    cli
    pushl $0
    pushl $15
    jmp isr_common_stub

/*  16: Coprocessor Fault Exception */
_isr16:
    cli
    pushl $0
    pushl $16
    jmp isr_common_stub

/*  17: Alignment Check Exception (486+) */
_isr17:
    cli
    pushl $0
    pushl $17
    jmp isr_common_stub

/*  18: Machine Check Exception (Pentium/586+) */
_isr18:
    cli
    pushl $0
    pushl $18
    jmp isr_common_stub

/*  19: Reserved */
_isr19:
    cli
    pushl $0
    pushl $19
    jmp isr_common_stub

/*  20: Reserved */
_isr20:
    cli
    pushl $0
    pushl $20
    jmp isr_common_stub
    
/*  21: Reserved */
_isr21:
    cli
    pushl $0
    pushl $21
    jmp isr_common_stub
    
/*  22: Reserved */
_isr22:
    cli
    pushl $0
    pushl $22
    jmp isr_common_stub
    
/*  23: Reserved */
_isr23:
    cli
    pushl $0
    pushl $23
    jmp isr_common_stub
    
/*  24: Reserved */
_isr24:
    cli
    pushl $0
    pushl $24
    jmp isr_common_stub
    
/*  25: Reserved */
_isr25:
    cli
    pushl $0
    pushl $25
    jmp isr_common_stub
    
/*  26: Reserved */
_isr26:
    cli
    pushl $0
    pushl $26
    jmp isr_common_stub

/*  27: Reserved */
_isr27:
    cli
    pushl $0
    pushl $27
    jmp isr_common_stub

/*  28: Reserved */
_isr28:
    cli
    pushl $0
    pushl $28
    jmp isr_common_stub

/*  29: Reserved */
_isr29:
    cli
    pushl $0
    pushl $29
    jmp isr_common_stub

/*  30: Reserved */
_isr30:
    cli
    pushl $0
    pushl $30
    jmp isr_common_stub

/*  31: Reserved */
_isr31:
    cli
    pushl $0
    pushl $31
    jmp isr_common_stub


/* 	We call a C function in here. We need to let the assembler know
	that '_fault_handler' exists in another file */
	
.extern _fault_handler

/* 	This is our common ISR stub. It saves the processor state, sets
	up for kernel mode segments, calls the C-level fault handler,
	and finally restores the stack frame. */
	
isr_common_stub:
    pusha
    pushw %ds
    pushw %es
    pushw %fs
    pushw %gs
    movb $0x10, %ax   ; Load the Kernel Data Segment descriptor!
    movb %ax, %ds
    movb %ax, %es
    movb %ax, %fs
    movb %ax, %gs
    movl %esp, %eax   ; Push us the stack
    pushl %eax
    movl _fault_handler, %eax
    call %eax       ; A special call, preserves the 'eip' register
    popl %eax
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8     ; Cleans up the pushed error code and pushed ISR number
    iret           ; pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP!
		
