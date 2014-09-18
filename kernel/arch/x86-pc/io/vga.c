/** 
 * @author Konstantin Tcholokachvili
 * @date 2007, 2013
 * @license MIT License
 * 
 * VGA screen handling functions 
 */ 

#include "vga.h"
#include <arch/x86-pc/io/keyboard.h>


struct vga_symbol
{
	/** Position X on the screen */
	uint8_t position_x;
	/** Position Y on the screen */
	uint8_t position_y; 
	/** Represents a foreground/background color set */
	uint8_t attributes;
};

struct vga_symbol symbol;


void vga_clear(void)
{
	uint8_t *video;

	for (video = (uint8_t*)SCREEN_START; 
		video <= (uint8_t*)SCREEN_PAGE_LIMIT; 
		video = video + 2)
	{
		*video = 0;
		*(video + 1) = 0x00; /* Set with a black background/foreground */
	}
	
	/* Move the cursor back to the starting point */
	symbol.position_x = 0;
	symbol.position_y = 0;
}


void vga_scroll_up(uint8_t nb_lines)
{
	uint8_t *video;
	uint8_t *tmp;
	
	for( video = (uint8_t*)SCREEN_START; 
		video < (uint8_t*)SCREEN_PAGE_LIMIT; 
		video++)
	{
		tmp = (uint8_t*)(video + nb_lines * COLUMNS * 2);
		
		if (tmp < (uint8_t*)SCREEN_PAGE_LIMIT)
			*video = *tmp;
		else
			*video = 0;
	}
	
	symbol.position_y -= nb_lines;
}


void vga_set_position(uint8_t x, uint8_t y)
{
	symbol.position_x = x;
	symbol.position_y = y;
}


void vga_set_attributes(uint8_t attributes)
{
	symbol.attributes = attributes;
}


void vga_display_character(uchar_t character)
{
	uint8_t* video;
	
	switch(character)
	{
		case KBD_CR_NL: /* Carriage return or new line */
			symbol.position_x = 0;
			symbol.position_y++;
			
			if (symbol.position_y > 24)
				vga_scroll_up(symbol.position_y-24);
			
			break;
			
		case KBD_BACKSPACE:
			if (symbol.position_x > 0)
			{
				symbol.position_x--;
				video = (uchar_t *)(SCREEN_START + 2 * symbol.position_x 
						+ COLUMNS * 2 * symbol.position_y);
				*video = ' ';
				*(video+1) = symbol.attributes;
			}
			
			break;
			
		case KBD_TABULATION:
			if (symbol.position_x + 4 > COLUMNS)
				; /* What to do? */
			else
				symbol.position_x += 4;
			
			break;
			
		default: /* Other characters */
			video = (uchar_t*)(SCREEN_START + 2 * symbol.position_x + 
					COLUMNS * 2 * symbol.position_y);
			*video = character;
			*(video+1) = symbol.attributes;

			symbol.position_x++;
		
			if (symbol.position_x > 79)
			{
				symbol.position_x = 0;
				symbol.position_y++;
			}
			
			if (symbol.position_y > 24)
				vga_scroll_up(symbol.position_y-24);			
	}
}

