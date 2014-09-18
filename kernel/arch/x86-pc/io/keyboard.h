#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

/**
 * @file keyboard.h
 * @author Konstantin Tcholokachvili
 * @date 2007, 2013
 * @license MIT License
 * 
 * Keyboard handling
 */

#include <arch/x86/registers.h>

#define KEYBOARD_DATA_PORT	0x60
#define KEYBOARD_COMMAND_PORT	0x64

#define KBD_CR_NL 	0x0a
#define KBD_BACKSPACE 	0x08
#define KBD_TABULATION	0x09

/** Handles keyboard
 *
 * @param r Registers status during the interrupt
 */
void keyboard_interrupt_handler(int number);

#endif // _KEYBOARD_H_
