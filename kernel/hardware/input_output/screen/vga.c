/** 
 * @file vga.c
 * @date 2007
 * @author Konstantin Tcholokachvili
 * VGA screen handling functions 
 */ 

#include "vga.h"
#include "../keyboard/keyboard.h"


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
 * @param param_nb_lines describes number of lines to be scrolled
 * @return None
 */
void vga_scroll_up(uint8_t param_nb_lines)
{
	uint8_t *video;
	uint8_t *tmp;
	
	for( video = (uint8_t*)SCREEN_START; 
		video < (uint8_t*)SCREEN_PAGE_LIMIT; 
		video++)
	{
		tmp = (uint8_t*)(video + param_nb_lines * COLUMNS * 2);
		
		if (tmp < (uint8_t*)SCREEN_PAGE_LIMIT)
			*video = *tmp;
		else
			*video = 0;
	}
	
	g_position_y = g_position_y - param_nb_lines;
}



void vga_set_position(uint8_t param_x, uint8_t param_y)
{
	g_position_x = param_x;
	g_position_y = param_y;
}



void vga_set_attributes(uint8_t param_attributes)
{
	g_attributes = param_attributes;
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
		__do_character(*str, g_attributes);
		str++;
	}
}


/**
 * @brief Displays a character
 * @param param_character
 * @return None
 */
void vga_display_character(uchar_t param_character)
{
	__do_character(param_character, g_attributes);
}


/**
 * @brief Treat special characters
 * @param param_character
 * @return None
 */
void __do_character(uchar_t param_character, uchar_t param_attributes)
{
	uint8_t* video;
	
	switch( param_character )
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
			*video = param_character;
			*(video+1) = param_attributes;

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
