extern h
extern stack
extern nb_stack_items

global stack_push
global stack_pop

global ignore
global interpret_number


stack_push:
	mov ebx, nb_stack_items
	mov ecx, [ebx]
	mov [stack + ecx * 4], eax
	inc word [ebx]
	ret

stack_pop:
	mov ebx, nb_stack_items
	mov ecx, [ebx]
	mov eax, [stack + ecx * 4]
	dec word [ebx]
	ret

ignore:
	ret

interpret_number:
	mov eax, [esp+4]
	shr eax, 5
	call stack_push
	ret



;static void ignore(const cell_t word);
;static void interpret_forth_word(const cell_t word);
;static void interpret_big_number(const cell_t number);
;static void create_word(cell_t word);
;static void compile_word(const cell_t word);
;static void compile_big_number(const cell_t number);
;static void compile_number(const cell_t number);
;static void compile_macro(const cell_t word);
;static void interpret_number(const cell_t number);
;static void variable_word(const cell_t word);

