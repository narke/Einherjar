#include <lib/libc.h>
#include <arch/x86-pc/io/keyboard.h>
#include <arch/x86-pc/io/vga.h>

#include "editor.h"

#define HIGHBIT 0x80000000L
#define MASK    0xffffffffL

#define STACK_SIZE 8

cell_t *blocks;
uint8_t nb_red_words;

/*
 * Stack macros
 */
#define stack_push(x) *(++tos) = x
#define stack_pop()   *(tos--)
#define nos           tos[-1]	// Next On Stack
#define start_of(x)   (&x[0])

/* Data stack */
cell_t stack[STACK_SIZE];
cell_t *tos = start_of(stack);	// Top Of Stack

static void handle_input(uchar_t c)
{
    static bool_t ctrl = FALSE;
    static char word[32];
    static uint8_t i = 0;

    switch(c)
    {
        case KEY_CTRL:
			ctrl = TRUE;
			break;

        case KEY_SPACE:
            word[i] = '\0';
            i = 0;
            break;

		default:
			if (ctrl == TRUE)
			{
				switch(keyboard_get_keymap(c))
				{
					case 'r':
						vga_set_attributes(FG_RED | BG_BLACK);
						break;

					case 'y':
						vga_set_attributes(FG_YELLOW | BG_BLACK);
						break;

					case 'g':
						vga_set_attributes(FG_GREEN | BG_BLACK);
						break;

					case 'c':
						vga_set_attributes(FG_CYAN | BG_BLACK);
						break;

					case 'p':
						vga_set_attributes(FG_MAGENTA | BG_BLACK);
						break;

					case 'o':
						vga_set_attributes(FG_WHITE | BG_BLACK);
						break;

					default:
						;
				}
			}
			else
			{
				// Avoid displaying character pressed along CTRL
				vga_display_character(keyboard_get_keymap(c));

                // Make a word from characters (it won't be patented ;-)
                word[i++] = keyboard_get_keymap(c);
			}
			ctrl = FALSE;
	}
}


/*
 * Packing and unpacking words
 */
char *code = " rtoeanismcylgfwdvpbhxuq0123456789j-k.z/;:!+@*,?";

int get_code_index(const char letter)
{
	// Get the index of a character in the 'code' sequence.
	return strchr(code, letter) - code;
}

cell_t pack(const char *word_name)
{
	int word_length, i, bits, length, letter_code, packed;

	word_length = strlen(word_name);
	packed      = 0;
	bits        = 28;

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

char *unpack(cell_t word)
{
	unsigned char nybble;
	static char text[16];
	unsigned int coded, bits, i;

	coded  = word;
	i      = 0;
	bits   = 32 - 4;
	coded &= ~0xf;

	memset(text, 0, 16);

	while (coded)
	{
		nybble = coded >> 28;
		coded  = (coded << 4) & MASK;
		bits  -= 4;

		if (nybble < 0x8)
		{
			text[i] += code[nybble];
		}
		else if (nybble < 0xc)
		{
			text[i] += code[(((nybble ^ 0xc) << 1) | ((coded & HIGHBIT) > 0))];
			coded    = (coded << 1) & MASK;
			bits    -= 1;
		}
		else
		{
			text[i] += code[(coded >> 29) + (8 * (nybble - 10))];
			coded    = (coded << 3) & MASK;
			bits    -= 3;
		}

		i++;
	}

	return text;
}

static void do_word(cell_t word)
{
	uint8_t color;

	color = (int)word & 0x0000000f;

	switch (color)
	{
		case 0:
			break;
		case 1:
		case 2:
		case 8:
			vga_set_attributes(FG_YELLOW | BG_BLACK);
			break;
		case 3:
			vga_set_attributes(FG_RED | BG_BLACK);
            // Display each newly defined word on a new line
            if (nb_red_words > 0)
                printf("\n");
            nb_red_words++;
			break;
		case 4:
		case 5:
		case 6:
			vga_set_attributes(FG_GREEN | BG_BLACK);
			break;
		case 7:
			vga_set_attributes(FG_CYAN | BG_BLACK);
			break;
		case 9:
		case 10:
		case 11:
		case 15:
			vga_set_attributes(FG_WHITE | BG_BLACK);
			break;
		case 12:
			vga_set_attributes(FG_MAGENTA | BG_BLACK);
			break;
		default:
			;
	}

	if (color == 2 || color == 5 || color == 6 || color == 8 || color == 15)
	{
		printf("%d ", word >> 5);
	}
	else
	{
		printf("%s ", unpack(word));
	}
}

void run_block(cell_t n)
{
	unsigned long start, limit, i;

    nb_red_words = 0;

	start = n * 256;     // Start executing block from here...
	limit = (n+1) * 256; // to this point.

	for (i = start; i < limit; i++)
	{
		// Is the end of block reached? If so return.
		if (blocks[i] == 0)
			return;

		do_word(blocks[i]);
	}
}

void load(void)
{
	cell_t n;

	n = stack_pop();
	run_block(n);
}

void editor(struct console *cons, uint32_t initrd_start)
{
	char buffer[256];
	uchar_t c;

	vga_clear();

	blocks = (cell_t *)initrd_start;

	// Load block 0
	stack_push(0);
	load();

	memset(buffer, 0, sizeof(buffer));

	while (1)
	{
		console_read(cons, &c, 1);
		handle_input(c);
	}
}
