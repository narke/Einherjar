#include <arch/x86-pc/io/vga.h>
#include <lib/libc.h>
#include <lib/queue.h>

#include "colorforth.h"

#define CODE_HEAP_SIZE (1024 * 100)	// 100 Kb
#define STACK_SIZE     42

#define FORTH_TRUE -1      // In Forth world -1 means true
#define FORTH_FALSE 0

struct word_entry
{
	cell_t                 name;
	void                  *code_address;
	void                  *codeword;
	LIST_ENTRY(word_entry) next;
};

typedef void (*FUNCTION_EXEC)(void);

/*
 * Stack macros
 */
#define stack_push(x) *(++tos) = x
#define stack_pop()   *(tos--)
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

LIST_HEAD(, word_entry) forth_dictionary;
LIST_HEAD(, word_entry) macro_dictionary;

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
static void literal(void);

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

void
NEXT(void)
{
	IP++;
	((FUNCTION_EXEC)*IP)();
}

/*
 * Built-in words
 */
void comma(void)
{
	*h = stack_pop();
	h++;
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

void exit_word(void)
{
	cell_t n = rpop();
	(void)n;
}

void add(void)
{
	cell_t n = stack_pop();
	*tos += n;
}

void not(void)
{
	cell_t n = stack_pop();
	*tos -= n;
}

void multiply(void)
{
	cell_t n = stack_pop();
	*tos *= n;
}

void divide(void)
{
	cell_t n = stack_pop();
	*tos /= n;
}

void modulo(void)
{
	cell_t n = stack_pop();
	*tos %= n;
}

void lt(void)
{
	cell_t n = stack_pop();
	*tos = (*tos < n) ? FORTH_TRUE : FORTH_FALSE;
}

void gt(void)
{
	cell_t n = stack_pop();
	*tos = (*tos > n) ? FORTH_TRUE : FORTH_FALSE;
}

void ge(void)
{
	cell_t n = stack_pop();
	*tos = (*tos >= n) ? FORTH_TRUE : FORTH_FALSE;
}

void ne(void)
{
	cell_t n = stack_pop();
	*tos = (*tos != n) ? FORTH_TRUE : FORTH_FALSE;
}

void eq(void)
{
	cell_t n = stack_pop();
	*tos = (*tos == n) ? FORTH_TRUE : FORTH_FALSE;
}

void le(void)
{
	cell_t n = stack_pop();
	*tos = (*tos <= n) ? FORTH_TRUE : FORTH_FALSE;
}

void and(void)
{
	cell_t a = stack_pop();
	cell_t b = stack_pop();
	cell_t result = a & b;
	stack_push(result);
}

// It is xor actually.
void or(void)
{
	cell_t a = stack_pop();
	cell_t b = stack_pop();
	cell_t result = a ^ b;
	stack_push(result);

}

void dup_word(void)
{
	cell_t n = *tos;
	stack_push(n);

	NEXT();
}

void drop(void)
{
	(void)stack_pop(); // Cast to avoid a warning about not used computed value
}

void over(void)
{
	cell_t n = nos;
	stack_push(n);
}

void swap(void)
{
	cell_t t = *tos;
	*tos = nos;
	nos = t;
}

void dot_s(void)
{
	int i, nb_items;

	nb_items = tos - start_of(stack);

	vga_set_position(0, 22);
	vga_set_attributes(FG_YELLOW | BG_BLACK);
	printf("\nStack: ");

	for (i = 1; i < nb_items + 1; i++)
		printf("%d ", (int)stack[i]);
	printf("\n");
}

void store(void)
{
	cell_t address = stack_pop();
	cell_t value   = stack_pop();

	*(cell_t *)address = value;
}

void fetch(void)
{
	cell_t address = stack_pop();
	cell_t value   = *(cell_t *)address;

	stack_push(value);
}

void here(void)
{
	stack_push((cell_t)h);
}

void zero_branch(void)
{
	cell_t n = stack_pop();

	if (n == FORTH_TRUE)
		IP++;
	else
		IP = (unsigned long *)*IP;

	NEXT();
}

void if_(void)
{
	stack_push((cell_t)zero_branch);
	comma();

	here();
	stack_push(0);
	comma();
}

void then(void)
{
	here();
	swap();
	store();
}

void for_aux(void)
{
	cell_t n = stack_pop();
	rpush(n);

	NEXT();
}

void next_aux(void)
{
	cell_t n = rpop();
	cell_t addr = rpop();

	rpush(addr);

	n--;
	rpush(n);

	if (n > 0)
	{
		IP = (unsigned long *)addr;
		((FUNCTION_EXEC)*IP)();
	}
}

void for_(void)
{
	stack_push((cell_t)for_aux);
	comma();

	rpush((cell_t)h);
}

void next_(void)
{
	stack_push((cell_t)next_aux);
	comma();
}

void rdrop(void)
{
	(void)rpop();
}

void dot(void)
{
	printf("%d ", (int)stack_pop());
}

void i_word(void)
{
	cell_t n;

	n = rpop();
	stack_push(n);
}


/*
 * Helper functions
 */
void
do_word(const cell_t word)
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
		do_word(blocks[i]);
	}
}

struct word_entry *
lookup_word(cell_t name, const bool_t force_dictionary)
{
	struct word_entry *item;

	name &= 0xfffffff0; // Don't care about the color byte

	if (force_dictionary == FORTH_DICTIONARY)
	{
		LIST_FOREACH(item, &forth_dictionary, next)
		{
			if (name == item->name)
				return item;
		}
	}
	else
	{
		LIST_FOREACH(item, &macro_dictionary, next)
		{
			if (name == item->name)
				return item;
		}
	}

	return NULL;
}

static void
insert_builtins_into_forth_dictionary(void)
{
	struct word_entry *_comma, *_load, *_loads, *_forth, *_macro,
			  *_exit_word, *_store, *_fetch, *_add, *_not,
			  *_mult, *_div, *_ne, *_dup, *_dot, *_here, *_i;

	_comma		= malloc(sizeof(struct word_entry));
	_load		= malloc(sizeof(struct word_entry));
	_loads		= malloc(sizeof(struct word_entry));
	_forth		= malloc(sizeof(struct word_entry));
	_macro		= malloc(sizeof(struct word_entry));
	_exit_word	= malloc(sizeof(struct word_entry));
	_store		= malloc(sizeof(struct word_entry));
	_fetch		= malloc(sizeof(struct word_entry));
	_add		= malloc(sizeof(struct word_entry));
	_not		= malloc(sizeof(struct word_entry));
	_mult		= malloc(sizeof(struct word_entry));
	_div		= malloc(sizeof(struct word_entry));
	_ne		= malloc(sizeof(struct word_entry));
	_dup		= malloc(sizeof(struct word_entry));
	_dot		= malloc(sizeof(struct word_entry));
	_here		= malloc(sizeof(struct word_entry));
	_i		= malloc(sizeof(struct word_entry));

	_comma->name		= pack(",");
	_comma->code_address	= comma;
	_comma->codeword	= &(_comma->code_address);

	_load->name		= pack("load");
	_load->code_address	= load;
	_load->codeword		= &(_load->code_address);

	_loads->name		= pack("loads");
	_loads->code_address	= loads;
	_loads->codeword	= &(_loads->code_address);

	_forth->name		= pack("forth");
	_forth->code_address	= forth;
	_forth->codeword	= &(_forth->code_address);

	_macro->name		= pack("macro");
	_macro->code_address	= macro;
	_macro->codeword	= &(_macro->code_address);

	_exit_word->name		= pack(";");
	_exit_word->code_address	= exit_word;
	_exit_word->codeword		= &(_exit_word->code_address);

	_store->name		= pack("!");
	_store->code_address	= store;
	_store->codeword	= &(_store->code_address);

	_fetch->name		= pack("@");
	_fetch->code_address	= fetch;
	_fetch->codeword	= &(_fetch->code_address);

	_add->name		= pack("+");
	_add->code_address	= add;
	_add->codeword		= &(_add->code_address);

	_not->name		= pack("-");
	_not->code_address	= not;
	_not->codeword		= &(_not->code_address);

	_mult->name		= pack("*");
	_mult->code_address	= multiply;
	_mult->codeword		= &(_mult->code_address);

	_div->name		= pack("/");
	_div->code_address	= divide;
	_div->codeword		= &(_div->code_address);

	_ne->name		= pack("ne");
	_ne->code_address	= ne;

	_dup->name		= pack("dup");
	_dup->code_address	= dup_word;
	_dup->codeword		= &(_dup->code_address);

	_dot->name		= pack(".");
	_dot->code_address	= dot;
	_dot->codeword		= &(_dot->code_address);

	_here->name		= pack("here");
	_here->code_address	= here;
	_here->codeword		= &(_here->code_address);

	_i->name		= pack("i");
	_i->code_address	= i_word;
	_i->codeword		= &(_i->code_address);

	LIST_INSERT_HEAD(&forth_dictionary, _comma,	next);
	LIST_INSERT_HEAD(&forth_dictionary, _load,	next);
	LIST_INSERT_HEAD(&forth_dictionary, _loads,	next);
	LIST_INSERT_HEAD(&forth_dictionary, _forth,	next);
	LIST_INSERT_HEAD(&forth_dictionary, _macro,	next);
	LIST_INSERT_HEAD(&forth_dictionary, _exit_word,	next);
	LIST_INSERT_HEAD(&forth_dictionary, _store,	next);
	LIST_INSERT_HEAD(&forth_dictionary, _fetch,	next);
	LIST_INSERT_HEAD(&forth_dictionary, _add,	next);
	LIST_INSERT_HEAD(&forth_dictionary, _not,	next);
	LIST_INSERT_HEAD(&forth_dictionary, _mult,	next);
	LIST_INSERT_HEAD(&forth_dictionary, _div,	next);
	LIST_INSERT_HEAD(&forth_dictionary, _ne,	next);
	LIST_INSERT_HEAD(&forth_dictionary, _dup,	next);
	LIST_INSERT_HEAD(&forth_dictionary, _dot,	next);
	LIST_INSERT_HEAD(&forth_dictionary, _here,	next);
	LIST_INSERT_HEAD(&forth_dictionary, _i,		next);
}

static void
insert_builtins_into_macro_dictionary(void)
{
	struct word_entry *_rdrop, *_ne, *_swap, *_if, *_then, *_for, *_next;

	_rdrop = malloc(sizeof(struct word_entry));
	_ne = malloc(sizeof(struct word_entry));
	_swap = malloc(sizeof(struct word_entry));
	_if = malloc(sizeof(struct word_entry));
	_then = malloc(sizeof(struct word_entry));
	_for = malloc(sizeof(struct word_entry));
	_next = malloc(sizeof(struct word_entry));

	_rdrop->name               = pack("rdrop");
	_rdrop->code_address       = rdrop;
	_rdrop->codeword           = &(_rdrop->code_address);

	_ne->name                  = pack("ne");
	_ne->code_address          = ne;
	_ne->codeword              = &(_ne->code_address);

	_swap->name                = pack("swap");
	_swap->code_address        = swap;
	_swap->codeword            = &(_swap->code_address);

	_if->name                  = pack("if");
	_if->code_address          = if_;
	_if->codeword              = &(_if->code_address);

	_then->name                = pack("then");
	_then->code_address        = then;
	_then->codeword            = &(_then->code_address);

	_for->name                 = pack("for");
	_for->code_address         = for_;
	_for->codeword             = &(_for->code_address);

	_next->name                = pack("next");
	_next->code_address        = next_;
	_next->codeword            = &(_next->code_address);

	LIST_INSERT_HEAD(&macro_dictionary, _rdrop,	next);
	LIST_INSERT_HEAD(&macro_dictionary, _ne,	next);
	LIST_INSERT_HEAD(&macro_dictionary, _swap,	next);
	LIST_INSERT_HEAD(&macro_dictionary, _if,	next);
	LIST_INSERT_HEAD(&macro_dictionary, _then,	next);
	LIST_INSERT_HEAD(&macro_dictionary, _for,	next);
	LIST_INSERT_HEAD(&macro_dictionary, _next,	next);
}

void
literal(void)
{
	cell_t n;

	// Skip literal's address
	IP++;

	// Fetch the number from the next cell
	n = *(cell_t *)IP;
	n >>= 5;  // Make it a number again ;-)

	// Push the number on the stack
	stack_push(n);

	NEXT();
}

void
variable(void)
{
	IP++; // Fetch the variable's address from the next cell
	stack_push((cell_t)IP); // Push it on the stack
}

static void
execute(const struct word_entry *word)
{
	IP = word->code_address;
	((FUNCTION_EXEC)*(unsigned long *)word->codeword)();
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
	struct word_entry *entry;

	entry = lookup_word(word, FORTH_DICTIONARY);

	if (entry)
		execute(entry);
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
	struct word_entry *entry;

	entry = lookup_word(word, MACRO_DICTIONARY);

	if (entry)
	{
		// Execute macro word
		execute(entry);
	}
	else
	{
		entry = lookup_word(word, FORTH_DICTIONARY);

		if (entry)
		{
			// Compile a call to that word
			stack_push((cell_t)entry->code_address);
			comma();
		}
	}
}

static void
compile_number(const cell_t number)
{
	stack_push((cell_t)literal);
	comma();

	stack_push(number);
	comma();
}

static void
compile_big_number(const cell_t number)
{
	(void)number;
}

static void
compile_macro(const cell_t word)
{
	struct word_entry *entry;

	entry = lookup_word(word, MACRO_DICTIONARY);

	if (entry)
	{
		// Compile a call to that macro
		stack_push((cell_t)entry->code_address);
		comma();
	}
}

static void
create_word(cell_t word)
{
	struct word_entry *entry;

	entry = malloc(sizeof(struct word_entry));
	memset(entry, 0, sizeof(struct word_entry));

	if (!entry)
	{
		printf("Error: Not enough memory!\n");
		colorforth_finalize();
		return;
	}

	word &= 0xfffffff0;

	entry->name         = word;
	entry->code_address = h;
	entry->codeword     = h;

	if (selected_dictionary == MACRO_DICTIONARY)
		LIST_INSERT_HEAD(&macro_dictionary, entry, next);
	else
		LIST_INSERT_HEAD(&forth_dictionary, entry, next);
}

static void
variable_word(const cell_t word)
{
	// A variable must be defined in forth dictionary
	forth();

	create_word(word);

	// Variable's handler
	stack_push((cell_t)variable);
	comma();

	// The default value of a variable is 0 (green number)
	stack_push(0);
	comma();
}

/*
 * Initializing and deinitalizing colorForth
 */
void
colorforth_initialize(void)
{
	code_here = malloc(CODE_HEAP_SIZE);

	if (!code_here)
	{
		printf("Error: Not enough memory!\n");
		// FIXME exit();
	}

	h = code_here;

	LIST_INIT(&forth_dictionary);
	LIST_INIT(&macro_dictionary);

	// FORTH is the default dictionary
	forth();

	insert_builtins_into_forth_dictionary();
	insert_builtins_into_macro_dictionary();
}

void
colorforth_finalize(void)
{
	struct word_entry *item;

	while ((item = LIST_FIRST(&forth_dictionary)))
	{
		LIST_REMOVE(item, next);
		free(item);
	}

	while ((item = LIST_FIRST(&macro_dictionary)))
	{
		LIST_REMOVE(item, next);
		free(item);
	}

	free(code_here);
}

