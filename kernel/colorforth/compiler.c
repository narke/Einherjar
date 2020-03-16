#include <arch/x86-pc/io/vga.h>
#include <lib/libc.h>

#include "colorforth.h"

#define HEAP_SIZE	(1024 * 100)	// 100 Kb
#define STACK_SIZE	42

#define FORTH_TRUE -1      // In Forth world -1 means true
#define FORTH_FALSE 0

typedef void (*FUNCTION_EXEC)(void);

/*
 * Stack macros
 */
#define stack_push(x) *(tos++) = x
#define stack_pop()   *(--tos)
#define nos           tos[-1]	// Next On Stack
#define rpush(x)      *(++rtos) = x
#define rpop()        *(rtos--)
#define start_of(x)   (&x[0])

/* Data stack */
cell_t stack[STACK_SIZE];
cell_t *tos = start_of(stack);	// Top Of Stack

/* Return stack */
unsigned long rstack[STACK_SIZE];
unsigned long *rtos = start_of(rstack);

/*
 * Global variables
 */
unsigned long *code_here;
unsigned long *h;			// Code is inserted here
bool_t         selected_dictionary;
extern cell_t *blocks;			// Manage looping over the code contained in blocks
unsigned long *IP;			// Instruction Pointer
bool_t is_hex = FALSE;

/*
 * Prototypes
 */
static void ignore(const cell_t word);
static void interpret_forth_word(const cell_t word);
static void interpret_big_number(const cell_t number);
static void create_word(cell_t word);
static void compile_word(const cell_t word);
static void compile_big_number(const cell_t number);
static void compile_number(const cell_t number);
static void compile_macro(const cell_t word);
static void interpret_number(const cell_t number);
static void variable_word(const cell_t word);

/* Word extensions (0), comments (9, 10, 11, 15), compiler feedback (13)
 * and display macro (14) are ignored. */
void (*color_word_action[16])() = {ignore, interpret_forth_word,
	interpret_big_number, create_word, compile_word, compile_big_number,
	compile_number, compile_macro, interpret_number,
	ignore, ignore, ignore, variable_word, ignore, ignore, ignore};

/*
 * Built-in words
 */
void comma(void)
{
}

void load(void)
{
	cell_t n = stack_pop();
	run_block(n);
}

void loads(void)
{
	cell_t i, j;

	j = stack_pop();
	i = stack_pop();

	// Load blocks, excluding shadow blocks
	for (; i <= j; i += 2)
	{
		stack_push(i);
		load();
	}
}

void forth(void)
{
	selected_dictionary = FORTH_DICTIONARY;
}

void macro(void)
{
	selected_dictionary = MACRO_DICTIONARY;
}

void add(void)
{
	cell_t a = stack_pop();
	cell_t b = stack_pop();
	stack_push(a + b);
}

void divide(void)
{
	cell_t a = stack_pop();
	cell_t b = stack_pop();
	stack_push(b / a);
}

void dot_s(void)
{
	erase_stack();
	vga_set_position(0, 22);
	vga_set_attributes(FG_YELLOW | BG_BLACK);

	int nb_items = tos - start_of(stack);

	printf("\nStack [%d]: ", nb_items);

	for (int i = 0; i < nb_items; i++)
	{
		if (is_hex)
			printf("%x ", stack[i]);
		else
			printf("%d ", stack[i]);
	}

	printf("\n");
}

void dot(void)
{
	printf("%d ", (int)stack_pop());
}

/*
 * Helper functions
 */
void
dispatch_word(const cell_t word)
{
	uint8_t color = (int)word & 0x0000000f;
	(*color_word_action[color])(word);
}

void
run_block(const cell_t n)
{
	unsigned long start = n * 256;     // Start executing block from here...
	unsigned long limit = (n+1) * 256; // ...to this point.

	for (unsigned long i = start; i < limit-1; i++)
	{
		dispatch_word(blocks[i]);
	}
}

word_t forth_dictionary[128] =
{
	{.name = 0xfc000000, .code_address = comma},
	{.name = 0xa1ae0000, .code_address = load},
	{.name = 0xa1ae0800, .code_address = loads},
	{.name = 0xb1896400, .code_address = forth},
	{.name = 0x8ac84c00, .code_address = macro},
	{.name = 0xea000000, .code_address = dot},
	{.name = 0xf6000000, .code_address = add},
	{.name = 0xee000000, .code_address = divide},
	{0, 0},
};

word_t macro_dictionary[32] =
{
	{0, 0}
};

word_t
lookup_word(cell_t name, const bool_t force_dictionary)
{
	name &= 0xfffffff0; // Don't care about the color byte

	if (force_dictionary == FORTH_DICTIONARY)
	{
		for (int i = 0; forth_dictionary[i].name; i++)
		{
			if (name == forth_dictionary[i].name)
				return forth_dictionary[i];
		}
	}
	else
	{
		for (int i = 0; macro_dictionary[i].name; i++)
		{
			if (name == macro_dictionary[i].name)
				return macro_dictionary[i];
		}

	}

	return (word_t){0, 0};
}

static void
execute(const word_t word)
{
	IP = word.code_address;
	((FUNCTION_EXEC)word.code_address)();
}

/*
 * Colorful words handling
 */
static void
ignore(const cell_t word)
{
	(void)word; // Avoid an useless warning and do nothing!
}

static void
interpret_forth_word(const cell_t word)
{
	word_t found_word = lookup_word(word, FORTH_DICTIONARY);

	if (found_word.name)
	{
		execute(found_word);
	}
}

static void
interpret_big_number(const cell_t number)
{
	(void)number;
}

static void
compile_word(const cell_t word)
{
	(void)word;
}

static void
compile_number(const cell_t number)
{
	(void)number;
}

static void
compile_big_number(const cell_t number)
{
	(void)number;
}

static void
interpret_number(const cell_t number)
{
	stack_push(number >> 5);
}

static void
compile_macro(const cell_t word)
{
	(void)word;
}

static void
create_word(cell_t word)
{
	(void)word;
}

static void
variable_word(const cell_t word)
{
	(void)word;
}

/*
 * Initializing and deinitalizing colorForth
 */
void
colorforth_initialize(void)
{
	code_here = malloc(HEAP_SIZE);

	if (!code_here)
	{
		panic("Error: Not enough memory!\n");
	}

	h = code_here;

	// Init stack
	memset(stack, 0, STACK_SIZE);

	// FORTH is the default dictionary
	forth();
}
