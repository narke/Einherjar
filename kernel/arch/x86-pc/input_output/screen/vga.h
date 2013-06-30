/** 
 * @file vga.h
 * @author Konstantin Tcholokachvili
 * An VGA-capable screen and constants definitions
 * @date 2007
 * @see http://sos.enix.org [FR]
 * @see http://webster.cs.ucr.edu/AoA/DOS/ch23/CH23-1.html [EN]
 * @see http://my.execpc.com/~geezer [EN] 
 */ 

#ifndef _VGA_H_
#define _VGA_H_

/** Video RAM starting adress */ 
#define SCREEN_START 		0xB8000
/** Video screen page size: 4000 bytes or 4 Ko */
#define SCREEN_PAGE_SIZE	0xFA0 	
/** Video screen page maximal offset = SCREEN_START + SCREEN_PAGE_SIZE = 0xB8000 + 0xFA0 = 0xB8FA0 */
#define SCREEN_PAGE_LIMIT (SCREEN_START + SCREEN_PAGE_SIZE) 

/** Number of maximal lines on a VGA screen */
#define LINES 	25
/** Number of maximal columns on a VGA screen */
#define COLUMNS 80

/** VGA control register */
#define VGA_CONTROL_REGISTER 	0x3D4
/** VGA data register */
#define VGA_DATA_REGISTER 	0x3D5

/* Normal and Dark/Light foreground colors */
#define FG_BLACK           0
#define FG_BLUE            1
#define FG_GREEN           2
#define FG_CYAN            3
#define FG_RED             4
#define FG_MAGENTA         5
#define FG_BROWN           6
#define FG_WHITE           7
#define FG_DARK_GRAY       8
#define FG_BRIGHT_BLUE     9
#define FG_BRIGHT_GREEN    10
#define FG_BRIGHT_CYAN     11
#define FG_PINK            12
#define FG_BRIGHT_MAGENTA  13
#define FG_YELLOW          14
#define FG_BRIGHT_WHITE    15

/* Background colors */
#define BG_BLACK     (FG_BLACK	 << 4)
#define BG_BLUE      (FG_BLUE 	 << 4)
#define BG_GREEN     (FG_GREEN 	 << 4)
#define BG_CYAN      (FG_CYAN 	 << 4)
#define BG_RED       (FG_RED 	 << 4)
#define BG_MAGENTA   (FG_MAGENTA << 4)
#define BG_BROWN     (FG_BROWN 	 << 4)
#define BG_LTGRAY    (FG_WHITE 	 << 4)

/* Blinking */
#define FG_BLINKING  (1 << 7)

#include <arch/all/types.h>
 
/** VGA-capable screen driver */
	
/** Clear the screen (by setting it black) */
void vga_clear(void);

/** Scroll up the screen by a given number of lines */
void vga_scroll_up(uint8_t param_nb_lines);

/** Set the X and Y position where the next string would be displayed */
void vga_set_position(uint8_t param_x, uint8_t param_y);

/** Set attributes: foreground/background colors and blinking */
void vga_set_attributes(uint8_t param_attributes);

/** Displays a formatted string */							
void vga_display_string(const char* str);

/** Displays a character */
void vga_display_character(uchar_t param_character);

/** character display helper function */
void __do_character(uchar_t param_character, uchar_t param_attributes);


#endif
