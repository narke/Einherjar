#include <arch/x86-pc/io/vga.h>
#include <lib/libc.h>

#include "colorforth.h"

#define HEAP_SIZE (1024 * 100)	// 100 Kb
#define STACK_SIZE     42

#define FORTH_TRUE -1      // In Forth world -1 means true
#define FORTH_FALSE 0

/*
 * Context
 */
typedef struct
{
	long *stack_ptr;		// Offset 0
	unsigned char *code_here;	// Offset 4
	unsigned char *data_here;	// Offset 8
	unsigned char *code_heap;	// Offset 12
	unsigned char *data_heap;	// Offset 16
	long stack[STACK_SIZE];		// Offset 20
} compiler_context_t;

compiler_context_t ctx;

typedef void (*FUNCTION_EXEC)(compiler_context_t *);

/*
 * Stack
 */
static void
stack_push(long value)
{
	*++ctx.stack_ptr = value;
}

static long
stack_pop(void)
{
	return *ctx.stack_ptr--;
}

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
 * Packing and unpacking words
 */
char *codes = " rtoeanismcylgfwdvpbhxuq0123456789j-k.z/;:!+@*,?";

int
get_code_index(const char letter)
{
	// Get the index of a character in the 'codes' sequence.
	return strchr(codes, letter) - codes;
}

#define HIGHBIT 0x80000000L
#define MASK    0xffffffffL


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

void dot_s(void)
{
	int i, nb_items;

	nb_items = ctx.stack_ptr - &ctx.stack[0];

	vga_set_position(0, 22);
	vga_set_attributes(FG_YELLOW | BG_BLACK);
	printf("\nStack: ");

	for (i = 1; i < nb_items + 1; i++)
	{
		if (is_hex)
			printf("%x ", (int)ctx.stack[i]);
		else
			printf("%d ", (int)ctx.stack[i]);
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
	unsigned long start, limit, i;

	start = n * 256;     // Start executing block from here...
	limit = (n+1) * 256; // ...to this point.

	for (i = start; i < limit-1; i++)
	{
		dispatch_word(blocks[i]);
	}
}

struct colorforth_word builtins[7] =
{
	{.name = 0xfc000000, .code_address = comma},
	{.name = 0xa1ae0000, .code_address = load},
	{.name = 0xa1ae0800, .code_address = loads},
	{.name = 0xb1896400, .code_address = forth},
	{.name = 0x8ac84c00, .code_address = macro},
	{.name = 0xea000000, .code_address = dot},
	{0, 0},
};

struct colorforth_word forth_dictionary[128] =
{
	{0, 0},
};

struct colorforth_word macro_dictionary[32] =
{
	{0, 0}
};

struct colorforth_word
lookup_word(cell_t name, const bool_t force_dictionary)
{
	int i;

	name &= 0xfffffff0; // Don't care about the color byte

	if (force_dictionary == FORTH_DICTIONARY)
	{
		for (i = 0; forth_dictionary[i].name; i++)
		{
			if (name == forth_dictionary[i].name)
				return forth_dictionary[i];
		}
	}
	else if (force_dictionary == MACRO_DICTIONARY)
	{
		for (i = 0; macro_dictionary[i].name; i++)
		{
			if (name == macro_dictionary[i].name)
				return macro_dictionary[i];
		}

	}
	else if (force_dictionary == BUILTINS_DICTIONARY)
	{
		for (i = 0; builtins[i].name; i++)
		{
			if (name == builtins[i].name)
				return builtins[i];
		}

	}

	return (struct colorforth_word){0, 0};
}

/*
 * Colorful words handling
 */
static void
ignore(const cell_t word)
{
	(void)word;
}

static void
interpret_forth_word(const cell_t word)
{
	(void)word;
	/*struct colorforth_word found_word = lookup_word(word, FORTH_DICTIONARY);

	if (found_word.name)
	{
		stack_push((cell_t)found_word.code_address);
		//((FUNCTION_EXEC)COLORFORTH_EXEC)(&ctx);
	}
	else
	{
		found_word = lookup_word(word, BUILTINS_DICTIONARY);

		if (found_word.name)
		{
			//((FUNCTION_EXEC)found_word.code_address)(&ctx);
		}
	}*/
}

static void
interpret_big_number(const cell_t number)
{
	(void)number;
}

static void
interpret_number(const cell_t number)
{
	stack_push(number >> 5);
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
		printf("Error: Not enough memory!\n");
		// FIXME exit();
	}

	h = code_here;

	// Init stack
	memset(&ctx, 0, sizeof(compiler_context_t));
	ctx.stack_ptr = &ctx.stack[0];

	// FORTH is the default dictionary
	forth();
}

void
colorforth_finalize(void)
{
	free(code_here);
}
