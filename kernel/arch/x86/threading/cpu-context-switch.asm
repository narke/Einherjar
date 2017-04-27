section .text

global cpu_context_switch
cpu_context_switch:
				; esp+64 arg2 = destination context
				; esp+60 arg1 = source context
				; esp+56 caller ip
	pushf			; esp+52 (eflags)
	push cs			; esp+48 (cs)
	push dword resume_pc	; esp+44 (ip)
	push 0			; esp+40 (error code)
	push ebp		; esp+36
	push edi		; esp+32
	push esi		; esp+28
	push edx		; esp+24
	push ecx		; esp+20
	push ebx		; esp+16
	push eax		; esp+12
	sub esp, 2		; esp+10 (alignment)
	o16 push ss		; esp+8
	o16 push ds		; esp+6
	o16 push es		; esp+4
	o16 push fs		; esp+2
	o16 push gs		; esp

	; Store the address of the saved context
	mov ebx, [esp+60]
	mov [ebx], esp

	; Switching context by changing stack
	mov esp, [esp+64]

	; Restore CPU's context
	o16 pop gs
	o16 pop fs
	o16 pop es
	o16 pop ds
	o16 pop ss
	add esp, 2
	pop eax
	pop ebx
	pop ecx
	pop edx
	pop esi
	pop edi
	pop ebp
	add esp, 4		; Ignore "error code"

	; Restores eflags, cs and eip registers
	iret			; Equivalent to: popfl ; ret

resume_pc:
	; Bring back the context prior to cpu_context_switch call
	; esp+8	arg2 = destination context
	; esp+4	arg1 = source context
	; esp	caller ip
	ret
