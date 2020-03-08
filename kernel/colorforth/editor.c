/*
 * Credits: unpack(), print_hex() and print_dec() are courtesy of
 * Andrei Dragomir.
 */

#include <lib/libc.h>
#include <arch/x86-pc/io/keyboard.h>
#include <arch/x86-pc/io/vga.h>

#include "colorforth.h"

#define HIGHBIT 0x80000000L
#define MASK    0xffffffffL

#define BLOCK_SIZE 1024

#define INTERPRET_NUMBER_TAG 8
#define INTERPRET_WORD_TAG   0x00000001

cell_t *blocks;
cell_t nb_block;
unsigned int word_index;
uint32_t total_blocks;

static void command_prompt(void);
static void command_prompt_erase(void);

bool_t is_command = FALSE;
extern bool_t is_hex;

/* Prototype of a later implemented function */
static void display_block(cell_t n);

char hex[] = {
	'0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};

static void
do_cmd(char *word)
{
	cell_t packed;

	command_prompt_erase();

	if (is_number(word))
	{
		packed = ((atoi(word) << 5) & MASK) + INTERPRET_NUMBER_TAG;
	}
	else
	{
		vga_set_position(0, 18);
		packed = (pack(word) & 0xfffffff0 ) | INTERPRET_WORD_TAG;

		struct colorforth_word w = lookup_word(packed, FORTH_DICTIONARY);

		if (w.name == 0)
		{
			// Not found!
			vga_set_position(0, 22);
			vga_set_attributes(FG_PINK | BG_BLACK);
			printf("Error: Word not found!");
			command_prompt();
			return;
		}
	}

	dispatch_word(packed);
	dot_s();
	command_prompt();
}

static void
handle_input(uchar_t scancode)
{
	static bool_t escape = FALSE;
	static uint8_t i = 0;
	static char word[32];

	if (scancode == KEY_ESCAPE)
	{
		escape = TRUE;
		return;
	}
	else if (scancode == KEY_F1)
	{
		if (is_hex)
			is_hex = FALSE;
		else
			is_hex = TRUE;

		dot_s();
		return;
	}
	else if (scancode == KEY_F2)
	{
		is_command = TRUE;
		command_prompt();
		return;
	}

	if (escape)
	{
		escape = FALSE;

		switch(keyboard_get_keymap(scancode))
		{
			case 'r':
				vga_set_attributes(FG_RED | BG_BLACK);
				return;

			case 'y':
				vga_set_attributes(FG_YELLOW | BG_BLACK);
				return;

			case 'g':
				vga_set_attributes(FG_GREEN | BG_BLACK);
				return;

			case 'c':
				vga_set_attributes(FG_CYAN | BG_BLACK);
				return;

			case 'm':
				vga_set_attributes(FG_MAGENTA | BG_BLACK);
				return;

			case 'w':
				vga_set_attributes(FG_BRIGHT_WHITE | BG_BLACK);
				return;

			case 'e':
				is_command = FALSE;
				vga_set_position(0, 0);
				vga_update_cursor();
				return;

			default:
				;
		}
	}

	switch(scancode)
	{

		case KEY_SPACE:
			word[i] = '\0';
			i = 0;

			if (is_command)
				do_cmd(word);
			else
				vga_display_character(' ');
			break;

		case KEY_UP:
			vga_update_position(0, -1);
			vga_update_cursor();
			break;

		case KEY_DOWN:
			vga_update_position(0, 1);
			vga_update_cursor();
			break;

		case KEY_LEFT:
			vga_update_position(-1, 0);
			vga_update_cursor();
			break;

		case KEY_RIGHT:
			vga_update_position(1, 0);
			vga_update_cursor();
			break;

		case KEY_PAGE_UP:
			if (nb_block-1 == -1)
				break;

			display_block(--nb_block);
			break;

		case KEY_PAGE_DOWN:
			if (nb_block+1 > (cell_t)total_blocks-1)
				break;

			display_block(++nb_block);
			break;

		default:
			vga_display_character(keyboard_get_keymap(scancode));

			// Make a word from characters (it won't be patented ;-)
			word[i++] = keyboard_get_keymap(scancode);
	}
}


/*
 * Packing and unpacking words
 */
char *code = " rtoeanismcylgfwdvpbhxuq0123456789j-k.z/;:!+@*,?";

static int
get_code_index(const char letter)
{
	// Get the index of a character in the 'code' sequence.
	return strchr(code, letter) - code;
}

cell_t
pack(const char *word_name)
{
	unsigned int word_length, i, bits, length, letter_code, packed;

	word_length = strlen(word_name);
	assert(word_length != 0);

	packed = 0;
	bits   = 28;

	for (i = 0; i < word_length; i++)
	{
		letter_code = get_code_index(word_name[i]);
		length      = 4 + (letter_code > 7) + (2 * (letter_code > 15));
		letter_code += (8 * (length == 5)) + ((96 - 16) * (length == 7));
		packed      = (packed << length) + letter_code;
		bits        -= length;
	}

	packed <<= bits + 4;
	return packed;
}

char *
unpack(cell_t word)
{
	unsigned char nibble;
	static char text[16];
	unsigned int coded, bits, i;

	coded  = word;
	i      = 0;
	bits   = 32 - 4;
	coded &= ~0xf;

	memset(text, 0, 16);

	while (coded)
	{
		nibble = coded >> 28;
		coded  = (coded << 4) & MASK;
		bits  -= 4;

		if (nibble < 0x8)
		{
			text[i] += code[nibble];
		}
		else if (nibble < 0xc)
		{
			text[i] += code[(((nibble ^ 0xc) << 1) | ((coded & HIGHBIT) > 0))];
			coded    = (coded << 1) & MASK;
			bits    -= 1;
		}
		else
		{
			text[i] += code[(coded >> 29) + (8 * (nibble - 10))];
			coded    = (coded << 3) & MASK;
			bits    -= 3;
		}

		i++;
	}

	return text;
}

static void
print_hex(unsigned int i)
{
	int n = 8, f = 0;

	if (i == 0)
	{
		vga_display_character('0');
		return;
	}

	while (n--)
	{
		if (!(i & 0xf0000000))
		{
			if (f)
				vga_display_character('0');
		}
		else
		{
			f = 1;
			vga_display_character(hex [i >> 28]);
		}
		i <<= 4;
	}
}

static void
print_dec(int i)
{
	int j, k, f = 0;

	if (i == 0)
	{
		vga_display_character('0');
		return;
	}

	if (i < 0)
	{
		vga_display_character('-');
		i = -i;
	}

	for (j = 1000000000; j != 0; j /= 10)
	{
		k = i / j;

		if (k == 0)
		{
			if (f)
				vga_display_character('0');
		}
		else
		{
			i -= j * k;
			f = 1;
			vga_display_character(hex[k]);
		}
	}
}

static void
print_number(cell_t word, bool_t is_hex)
{
	if (is_hex)
		print_hex(word >> 5);
	else
		print_dec(word >> 5);
	vga_display_character(' ');
}

static void
display_word(cell_t word)
{
	uint8_t color = word & 0x0000000f;
	bool_t is_hex = FALSE;

	switch (color)
	{
		case 0:
			vga_update_position(-1, 0); // Go one character left to replace the blank space
			vga_update_cursor();
			printf("%s ", unpack(word));
			break;

		case 1:
			vga_set_attributes(FG_YELLOW | BG_BLACK);
			printf("%s ", unpack(word));
			break;

		case 2:
			vga_set_attributes(FG_BROWN | BG_BLACK);
			if (word & 0x10)
				print_hex(blocks[++word_index]);
			else
				print_dec(blocks[++word_index]);
			vga_display_character(' ');
			break;

		case 3:
			vga_set_attributes(FG_RED | BG_BLACK);
			printf("%s ", unpack(word));
			break;

		case 4:
			vga_set_attributes(FG_BRIGHT_GREEN | BG_BLACK);
			printf("%s ", unpack(word));
			break;

		case 5:
			vga_set_attributes(FG_GREEN | BG_BLACK);
			if (word & 0x10)
				print_hex(blocks[++word_index]);
			else
				print_dec(blocks[++word_index]);
			vga_display_character(' ');
			break;

		case 6:
			if (word & 0x10)
			{
				is_hex = TRUE;
				vga_set_attributes(FG_GREEN | BG_BLACK);
			}
			else
			{
				is_hex = FALSE;
				vga_set_attributes(FG_BRIGHT_GREEN | BG_BLACK);
			}
			print_number(word, is_hex);
			break;

		case 7:
			vga_set_attributes(FG_BRIGHT_CYAN | BG_BLACK);
			printf("%s ", unpack(word));
			break;

		case 8:
			if (word & 0x10)
			{
				is_hex = TRUE;
				vga_set_attributes(FG_BROWN | BG_BLACK);
			}
			else
			{
				is_hex = FALSE;
				vga_set_attributes(FG_YELLOW | BG_BLACK);
			}
			print_number(word, is_hex);
			break;

		case 9:
		case 10:
		case 11:
			vga_set_attributes(FG_BRIGHT_WHITE | BG_BLACK);
			printf("%s ", unpack(word));
			break;

		case 12:
			vga_set_attributes(FG_MAGENTA | BG_BLACK);
			printf("%s ", unpack(word));

			vga_set_attributes(FG_BRIGHT_GREEN | BG_BLACK);
			if (word & 0x10)
				print_hex(blocks[++word_index]);
			else
				print_dec(blocks[++word_index]);
			vga_display_character(' ');
			break;

		case 14:
			printf("\n");
			break;

		case 15:
			vga_set_attributes(FG_BRIGHT_WHITE | BG_BLACK);
			if (word & 0x10)
				is_hex = TRUE;
			else
				is_hex = FALSE;
			print_number(word, is_hex);
			break;

		default:
			;
	}
}

static void
status_bar_update_block_number(cell_t n)
{
	vga_set_position(65, 23);
	vga_set_attributes(FG_BRIGHT_GREEN | BG_BLACK);

	printf("Block: %d\n", n);

	vga_set_position(0, 0);
	vga_update_cursor();
}

void
erase_stack(void)
{
#define VGA_SCREEN 0xB8000
#define VGA_NB_COLUMNS 80
	uint8_t *video;

	// Go to the prompt line (23) and erase it along ther error
	// reporting area (24). Don't erase the prompt (hence 2).

	for (video = (uint8_t *)VGA_SCREEN + 23 * VGA_NB_COLUMNS * 2;
		video <= (uint8_t *)VGA_SCREEN + 24 * VGA_NB_COLUMNS * 2;
		video = video + 2)
	{
		*video = 0;
	}

	vga_set_position(2, 21);
	vga_update_cursor();
}

static void
display_command_prompt(void)
{
	vga_set_position(0, 21);
	vga_set_attributes(FG_BRIGHT_GREEN | BG_BLACK);
	printf("> ");
	dot_s();
}

static void
command_prompt(void)
{
	vga_set_position(2, 21);
	vga_set_attributes(FG_YELLOW | BG_BLACK);
	vga_update_cursor();
}

static void
command_prompt_erase(void)
{
#define VGA_SCREEN 0xB8000
#define VGA_NB_COLUMNS 80
	uint8_t *video;

	// Go to the prompt line (21) and erase it along ther error
	// reporting area (22). Don't erase the prompt (hence 2).

	for (video = (uint8_t *)VGA_SCREEN + 21 * VGA_NB_COLUMNS * 2 + 2;
		video <= (uint8_t *)VGA_SCREEN + 22 * VGA_NB_COLUMNS * 2 + VGA_NB_COLUMNS * 2 - 2;
		video = video + 2)
	{
		*video = 0;
	}

	vga_set_position(2, 21);
	vga_update_cursor();
}

static void
display_block(cell_t n)
{
	unsigned long start, limit;

	start = n * 256;     // Start executing block from here...
	limit = (n+1) * 256; // to this point.

	vga_clear();


	for (word_index = start; word_index < limit; word_index++)
	{
		display_word(blocks[word_index]);
	}

	display_command_prompt();
	status_bar_update_block_number(n);
}

void
editor(void *args)
{
	uchar_t c;
	struct editor_args *params;

	params = (struct editor_args *)args;

	blocks = (cell_t *)params->initrd_start;
	total_blocks = (params->initrd_end - params->initrd_start) / BLOCK_SIZE;

	vga_clear();

	display_block(0);

	while (1)
	{
		console_read(params->cons, &c, 1);
		handle_input(c);
	}
}
