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

// See scancodes at wiki.osdev.org/PS2/2_Keyboard
#define KEY_ESCAPE      0x01
#define KEY_BACKSPACE   0x0e
#define KEY_TAB         0x0f
#define KEY_ENTER       0x1c
#define KEY_LEFT_CTRL   0x1d
#define KEY_LEFT_SHIFT  0x2a
#define KEY_RIGHT_SHIFT 0x36
#define KEY_LEFT_ALT    0x38
#define KEY_SPACE       0x39
#define KEY_CAPS_LOCK   0x3a
#define KEY_F1          0x3b
#define KEY_F2          0x3c
#define KEY_F3          0x3d
#define KEY_F4          0x3e
#define KEY_F5          0x3f
#define KEY_F6          0x40
#define KEY_F7          0x41
#define KEY_F8          0x42
#define KEY_F9          0x43
#define KEY_F10         0x44
#define KEY_NUMBER_LOCK 0x45
#define KEY_SCROLL_LOCK 0x46
#define KEY_F11         0x57
#define KEY_F12         0x58
#define KEY_UP          0x48
#define KEY_DOWN        0x50
#define KEY_LEFT        0x4b
#define KEY_RIGHT       0x4d
#define KEY_PAGE_UP	0x49
#define KEY_PAGE_DOWN	0x51

/** Handles keyboard
 *
 * @param r Registers status during the interrupt
 */
void keyboard_interrupt_handler(int number);

void keyboard_setup(struct console *term);

char keyboard_get_keymap(uchar_t i);

#endif // _KEYBOARD_H_
