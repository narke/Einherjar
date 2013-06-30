/**
 * @file keyboard.h
 * @author Konstantin Tcholokachvili
 * @date 2007, 2013
 */

#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include <arch/x86/registers.h>

#define KEYBOARD_DATA_PORT	0x60
#define KEYBOARD_COMMAND_PORT	0x64

#define KBD_CR_NL 	0x0a
#define KBD_BACKSPACE 	0x08
#define KBD_TABULATION	0x09

void keyboard_interrupt_handler(struct regs *r);

#endif
