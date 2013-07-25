#ifndef _ISR_H_
#define _ISR_H_

/**
 * @file isr.h
 * @author Konstantin Tcholokachvili
 * @date 2013
 * @license MIT License
 *
 * Exceptions handling
 */

#include <arch/x86-all/registers.h>

/** Setup exceptions handling */
void x86_exceptions_setup(void);

/** Exception handler function */
void x86_exception_handler(struct regs *r);

#endif // _ISR_H_
