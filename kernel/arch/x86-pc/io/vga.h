#pragma once

/**
 * @file vga.h
 * @author Konstantin Tcholokachvili
 * @date 2007, 2013, 2016
 * @license MIT License
 *
 * @see http://sos.enix.org [FR]
 * @see http://webster.cs.ucr.edu/AoA/DOS/ch23/CH23-1.html [EN]
 * @see http://my.execpc.com/~geezer [EN]
 *
 * VGA-capable screen constants and functions
 */

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

#include <lib/types.h>

/** VGA-capable screen driver */

/** Clear the screen (by setting it to black) */
void vga_clear(void);

/** Update the cursor position */
void vga_update_cursor(void);

/** Scroll up the screen by a given number of lines
 *
 * @param nb_lines Number of lines to scroll up
 */
void vga_scroll_up(uint8_t nb_lines);

/** Set X and Y positions where the next string would be displayed
 *
 * @param x X position
 * @param y Y position
 */
void vga_set_position(uint8_t x, uint8_t y);

/** Update X and Y positions where the next string would be displayed
 *
 * @param x X position
 * @param y Y position
 */
void vga_update_position(int8_t x, int8_t y);

/** Set attributes: foreground/background colors and blinking
 *
 * @param attributes Attributes byte corresponding to foreground/background colors and blinking
 */
void vga_set_attributes(uint8_t attributes);


/** Displays a character
 *
 * @param character Character to display or a special character to handle
 */
void vga_display_character(uchar_t character);
