#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

/**
 * @file keyboard.h
 * @author Konstantin Tcholokachvili
 * @date 2007, 2013, 2014, 2016
 * @license MIT License
 *
 * Keyboard handling
 */

#include <arch/x86/registers.h>
#include <io/console.h>
#include <lib/types.h>

#define KEYBOARD_DATA_PORT      0x60
#define KEYBOARD_COMMAND_PORT   0x64

#define KBD_CR_NL       0x0a
#define KBD_BACKSPACE   0x08
#define KBD_TABULATION  0x09

#define KEY_CTRL    0x1C
#define KEY_SPACE   0x38
#define KEY_UP      0x47
#define KEY_DOWN    0x4f
#define KEY_LEFT    0x4a
#define KEY_RIGHT   0x4c

/** Handles keyboard
 *
 * @param r Registers status during the interrupt
 */
void keyboard_interrupt_handler(int number);

void keyboard_setup(struct console *term);

char keyboard_get_keymap(uchar_t i);

#endif // _KEYBOARD_H_
