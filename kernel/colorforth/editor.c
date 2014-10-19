#include <lib/libc.h>
#include <arch/x86-pc/io/keyboard.h>
#include <arch/x86-pc/io/vga.h>

#include "editor.h"

static void handle_input(uchar_t c)
{
	static bool_t ctrl = FALSE;

	switch(c)
	{
		case KEY_CTRL:
			ctrl = TRUE;
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
			}
			ctrl = FALSE;
	}
}

void editor(struct console *cons)
{
	char buffer[256];
	uchar_t c;

	while (1)
	{
		memset(buffer, 0, sizeof(buffer));

		console_read(cons, &c, 1);
		handle_input(c);
	}
}
