.file "cpu_context_switch.S"

.intel_syntax noprefix
.text

/**
 * C Function called by the routines below in order to tell the CPU
 * where will be the kernel stack (needed by the interrupt handlers)
 * when next_ctxt will come back into kernel mode.
 *
 * @see end of cpu_context.c
 */
        
.globl cpu_context_switch
.type cpu_context_switch, @function
cpu_context_switch:
						// esp+64 arg2 = to_context
						// esp+60 arg1 = from_context
						// esp+56 caller ip
	pushf				// esp+52 (eflags)
	push cs				// esp+48 (cs)
.att_syntax prefix
	pushl $resume_pc	// esp+44 (ip)
.intel_syntax noprefix
	push 0				// esp+40 (error code)
	push ebp			// esp+36
	push edi			// esp+32
	push esi			// esp+28
	push edx			// esp+24
	push ecx			// esp+20
	push ebx			// esp+16
	push eax			// esp+12
	sub esp, 2			// esp+10 (alignment)
	pushw ss			// esp+8
	pushw ds			// esp+6
	pushw es			// esp+4
	pushw fs			// esp+2
	pushw gs			// esp

	/*
	 * Now that the original eax/ebx are stored, we can use them safely
	 */
        
	// Store the address of the saved context 
	mov ebx, [esp+60]
	mov [ebx], esp

	// This is the proper context switch ! We change the stack here 
	mov esp, [esp+64]

	// Restore the CPU context
	popw gs
	popw fs
	popw es
	popw ds
	popw ss
	add esp, 2
	pop eax
	pop ebx
	pop ecx
	pop edx
	pop esi
	pop edi
	pop ebp
	add esp, 4 // Ignore "error code"

	// This restores the eflags, the cs and the eip registers
	iret // equivalent to: popfl ; ret

resume_pc:
        // Same context as that when cpu_context_switch got called
        // arg2 = to_context	-- esp+8
        // arg1 = from_context	-- esp+4
        // caller ip			-- esp
	ret

