/** 
 * @file vga.c
 * @author Konstantin Tcholokachvili
 * @date 2007, 2013
 * VGA screen handling functions 
 */ 

#include "vga.h"
#include <arch/x86-pc/input_output/keyboard/keyboard.h>


/** Position X on the screen */
uint8_t g_position_x;
/** Position Y on the screen */
uint8_t g_position_y; 
/** Represents a foreground/background color set */
uint8_t g_attributes; 


/**
 * @brief Clears the screen
 * @param None
 * @return None
 */
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
	g_position_x = 0;
	g_position_y = 0;
}



/**
 * @brief Scrolls up the screen by X lines
 * @param nb_lines describes number of lines to be scrolled
 * @return None
 */
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
	
	g_position_y = g_position_y - nb_lines;
}


/**
 * @brief Set the starting position to display a character
 * @param x Position X
 * @param y Position Y
 * @return None
 */
void vga_set_position(uint8_t x, uint8_t y)
{
	g_position_x = x;
	g_position_y = y;
}


/**
 * @brief Set the starting position to display a character
 * @param attributes Attributes of VGA: foreground color/background color/blinking
 * @return None
 */
void vga_set_attributes(uint8_t attributes)
{
	g_attributes = attributes;
}


/**
 * @brief Displays a string
 * @param str
 * @return None
 */
void vga_display_string(const char* str)
{
	while(*str != 0)
	{		
		vga_display_character(*str++);
	}
}


/**
 * @brief Displays a character
 * @param character
 * @return None
 */
void vga_display_character(uchar_t character)
{
	uint8_t* video;
	
	switch(character)
	{
		case KBD_CR_NL: /* Carriage return or new line */
			g_position_x = 0;
			g_position_y++;
			
			if ( g_position_y > 24 )
				vga_scroll_up(g_position_y-24);
			
			break;
			
		case KBD_BACKSPACE:
			if ( g_position_x > 0 )
				g_position_x--;
			
			break;
			
		case KBD_TABULATION:
			if ( g_position_x + 4 > COLUMNS )
				; /* What to do? */
			else
				g_position_x = g_position_x + 4;
			
			break;
			
		default: /* Other characters */
			video = (uchar_t*)(SCREEN_START + 2 * g_position_x + COLUMNS * 2 * g_position_y);
			*video = character;
			*(video+1) = g_attributes;

			g_position_x++;
		
			if( g_position_x > 79 )
			{
				g_position_x = 0;
				g_position_y++;
			}
			
			if( g_position_y > 24 )
				vga_scroll_up(g_position_y-24);			
	}
}
