; Konstantin Tcholokachvili 2016
; This isn't a compiler per see but in colorForth's tradition its called a
; compiler.
;
; Based on code made by :
; 2012 Oct 21 Howerd Oakford Ported to NASM format.
; Chuck Moore (inventor, MASM)
; Mark Slicker (ported to GNU Assembler)
; Peter Appelman (ported to NASM with qwerty mods) and others... Thanks!!!
; 28 Oct 2007 Mark Tillotson

; Register usage:
; 0 EAX: stack (1st number on Data stack): TOS
; 1 ECX: string counter, scratch
; 2 EDX: address register A, I/O port, scratch
; 3 EBX: unused
; 4 ESP: byte pointer to top of Return stack
; 5 EBP: unused
; 6 ESI: byte pointer to 2nd number on Data stack: NOS
; 7 EDI: dword pointer to next word to be interpreted


%macro	NEXT 1
	dec ecx
	jnz %1
%endmacro

%macro	DUP_	0
	lea esi,[esi-4]
	mov [esi], eax
%endmacro

%macro	DROP	0
	lodsd
%endmacro

global run_block

section .text

empty_dictionaries:
	mov dword [forths], ((forth1-forth0)/4)
	mov dword [macros], ((macro1-macro0)/4)
	ret

; Words definitions

semicolon:
	mov edx, [H]
	sub edx, byte 5
	cmp dword [list], edx
	jnz .semicolon
	cmp byte [edx], 0xe8
	jnz .semicolon
	inc byte [edx]			; jmp
	ret
.semicolon:  mov byte [5+edx], 0xc3	; ret
	inc dword [H]
	ret

then:
	mov [list], esp
	mov edx, [H]
	sub edx, eax
	mov [-1+eax], dl
	DROP
	ret

begin:
	mov [list], esp
here:
	DUP_
	mov eax, [H]
	ret

; optimized dup (if previous word ended in a drop, no drop or dup)
question_dup:
	mov edx, [H]
	dec edx
	cmp dword [list], edx
	jnz compile_dup
	cmp byte [edx], 0xad
	jnz compile_dup
	mov [H], edx
	ret

compile_dup:	mov edx, [H]
	mov dword [edx], 0x89fc768d
	mov byte [edx+4], 06
	add dword [H], byte +5
	ret

compile_drop:
	mov edx, [H]
	mov [list], edx
	mov byte [edx], 0xad	; drop opcode (lodsd)
	inc dword [H]
	ret

act:	nop

question_lit:
	mov edx, [H]
	lea edx, [edx-5]
	cmp dword [list], edx
	jnz .question_lit2
	cmp byte [edx], 0xb8
	jnz .question_lit2
	DUP_
	mov eax, [list+4]
	mov [list], eax
	mov eax, [1+edx]
	cmp dword [edx-5], 0x89fc768d	; dup
	jz .question_lit1
	mov [H], edx
	jmp compile_drop
.question_lit1:	add dword [H], byte -10		; flag nz
	ret
.question_lit2:   xor edx, edx  		; flag z
	ret

comma:
	mov ecx, 4
dcomma:	mov edx, [H]
	mov [edx], eax
	mov eax, [esi]		; drop
	lea edx, [ecx+edx]
	lea esi, [esi+4]
	mov [H], edx
	ret

comma1:
	mov ecx, 1
	jmp short dcomma

comma2:
	mov ecx, 2
	jmp short dcomma

comma3:
	mov ecx, 3
	jmp short dcomma

less:
	cmp [esi], eax
	jl .less		; flag nz
	xor ecx, ecx		; flag z
.less:	ret

jump:
	pop edx
	add edx, eax
	lea edx, [5+eax*4+edx]
	add edx, [-4+edx]
	DROP
	jmp edx

mark:				; 0x324
	mov ecx, [macros]
	mov [marker], ecx
	mov ecx, [forths]
	mov [marker+4], ecx
	mov ecx, [H]
	mov [marker+2*4], ecx
	ret

empty:
	mov ecx, [marker+2*4]
	mov [H], ecx
	mov ecx, [marker+4]
	mov [forths], ecx
	mov ecx, [marker]
	mov [macros], ecx
	mov dword [class], 0
	ret

; Load and compile source code block
load:
	shl eax, 10-2		; multiply block number by 1024/4=256
	push edi
	mov edi, eax
	DROP
interpret:
	mov edx, [edi*4]
	inc edi
	and edx, byte 0xf	; mask down to bottom 4 (color) bits
	call [spaces+edx*4]	; index into 1 of 16 routines
	jmp short interpret	; loop exited by ignore on 0 word

emit:	nop
edig:	nop
dot:	nop

sps:
	DUP_
	;mov eax, (spaces-start)/4 ; TODO
	ret

; Helpers

; routine executed when color bits=0 (extension word)
; 9, 10, 11, 13, 14 or 15 (comments or undefined)
ignore:
	test dword [-4+edi*4], -0xf	; check for 0 in color bits of next word
	jnz nul			; just return if not 0
	pop edi			; pop 2 return addresses to finish laoding source code
	pop edi			; and return to the command line interpreter
nul:	ret

; routine executed when color bits=1 (yellow word)
execute:
	mov dword [lit], alit
	DUP_
	mov eax, [-4+edi*4]	; get next word
	and eax, byte -0x10	; mask off color bits
	call find_forth_word
	jnz abort
	DROP
	jmp [forth2+ecx*4]

; compile a 32-bit number
; routine executed when color bits=2 (yellow 32-bit number)
num:
	mov dword [lit], alit
	DUP_
	mov eax, [edi*4]
	inc edi
	ret

; called dup
adup:
	DUP_
	ret

; find in macro dictionary
find_macro_word:
	mov ecx, [macros]
	push edi
	lea edi, [macro0-4+ecx*4]
	jmp short find

; find in forth dictionary
find_forth_word:
	mov ecx, [forths]
	push edi
	lea edi, [forth0-4+ecx*4]
find:	std
	repne scasd
	cld
	pop edi
	ret

alit:	mov dword [lit], adup
literal:			; compile TOS as a 32-bit number
	call question_dup
	mov edx, [list]
	mov [list+4], edx
	mov edx, [H]
	mov [list], edx
	mov byte [edx], 0xb8
	mov dword [1+edx], eax
	add dword [H], byte +5
	ret

abort:
abort1:
	mov dword [spaces+3*4], forthd
	mov dword [spaces+4*4], compile_word
	mov dword [spaces+5*4], compile_number
	mov dword [spaces+6*4], compile_short_number
	ret

sdefine:
	pop dword [adefine]
	ret

; routine to set macro as the current dictionary
macro_:
	call sdefine

; routine to start a macro definition (red word)
macrod:
	mov ecx, [macros]
	inc dword [macros]
	lea ecx, [macro0+ecx*4]
	jmp short forthdd

; routine to set forth as the current dictionary
forth:
	call sdefine

; routine to start a forth definition (red word)
forthd:
	mov ecx, [forths]
	inc dword [forths]
	lea ecx, [forth0+ecx*4]
forthdd:
	mov edx, [-4+edi*4]
	and edx, byte -0x10
	mov [ecx], edx
	mov edx, [H]
	mov [forth2-forth0+ecx], edx
	lea edx, [forth2-forth0+ecx]
	shr edx, 2
	mov [last], edx
	mov [list], esp
	mov dword [lit], adup
	test dword [class], -1
	jz .fthd
	jmp [class]
.fthd:	ret


; return value of a variable
variable_value:
	DUP_
	mov eax, [4+forth0+ecx*4]
	ret

variable:
	call forthd
	mov dword [forth2-forth0+ecx], variable_value
	inc dword [forths]		; dummy entry for source address
	mov [4+ecx], edi
	call macrod
	mov dword [forth2-forth0+ecx], .var
	inc dword [macros]
	mov [4+ecx], edi
	inc edi
	ret

; if a variable is invoked as a macro
.var:	call [lit]
	mov eax, [4+macro0+ecx*4]
	jmp short compile_short

; compile a number (green)
compile_number:
	call [lit]
	mov eax, [edi*4]
	inc edi
	jmp short compile_short

; compile a short number (green)
compile_short_number:
	call [lit]
	mov eax, [-4+edi*4]
	sar eax, 5
compile_short:
	call literal
	DROP
	ret

; handle green words that could be in either the macro or the forth dictionary
compile_word:
	call [lit]
	mov eax, [-4+edi*4]
	and eax, byte -0x10
	call find_macro_word
	jnz .comp_word
	DROP
	jmp [macro2+ecx*4]
.comp_word:
	call find_forth_word
	mov eax, [forth2+ecx*4]
compile_word1:
	jnz abort
	mov edx, [H]
	mov dword [list], edx
	mov byte [edx], 0xe8	; 'call' opcode
	add edx, byte +5
	sub eax, edx
	mov dword [-4+edx], eax
	mov [H], edx
	DROP
	ret

; compile a call to a macro word (cyan)
compile:
	call [lit]
	mov eax, [-4+edi*4]
	and eax, byte -0x10
	call find_macro_word
	mov eax, [macro2+ecx*4]
	jmp short compile_word1

; compile a 27-bit number
compile_27bit_number:
	mov dword [lit], alit
	DUP_
	mov eax, [-4+edi*4]
	sar eax, 5
	ret

run_block:
	pop eax			; get the block number to load
	call load		; load the block


section .data

extern initrd_start_address

lit:	dd adup
marker:	dd 0, 0, 0
H	dd initrd_start_address
last	dd 0
class	dd 0
list:	dd 0, 0
macros: dd 0
forths:	dd 0

; Dictionaries
; Values were obtained with pack.py (see tools directory)

macro0:
	dd 0xf0000000		; ';'
	dd 0xc19b1000		; dup
	dd 0xff833620		; ?dup
	dd 0xc0278800		; drop
	dd 0x2c88c000		; then
	dd 0xc6957600		; begin
macro1:
	times 128 dd 0

forth0:
	dd 0x8ac84c00		; MACRO
	dd 0xb1896400		; FORTH
	dd 0x59100000		; act
	dd 0xa1ae0000		; load
	dd 0xc8828000		; here
	dd 0xff472000		; ?lit
	dd 0xd7f80000		; 3,
	dd 0xd5f80000		; 2,
	dd 0xd3f80000		; 1,
	dd 0xfc000000		; ,
	dd 0xa2420000		; less
	dd 0xe59a3880		; jump
	dd 0x8a8f4000		; mark
	dd 0x48e22000		; empt
	dd 0x48b90000		; emit
	dd 0xc0f57200		; digit
	dd 0xea000000		; .
	dd 0x86200000		; sp

forth1:
	times 512 dd 0

macro2:
	dd semicolon
	dd compile_dup
	dd question_dup
	dd compile_drop
	dd then
	dd begin

	times 128 dd 0

forth2:
	dd macro_
	dd forth
	dd act
	dd load
	dd here
	dd question_lit
	dd comma3
	dd comma2
	dd comma1
	dd comma
	dd less
	dd jump
	dd mark
	dd empty
	dd emit
	dd edig
	dd dot
	dd sps

	times 512 dd 0


spaces:	dd ignore, execute, num

adefine: ; where definitions go, either in macrod (dictionary) or forthd
	dd forthd	; default, the forth dictionary
	dd compile_word, compile_number, compile_short_number, compile
	dd compile_27bit_number, nul, nul, nul
	dd variable, nul, nul, nul

