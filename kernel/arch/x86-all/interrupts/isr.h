/**
 * @file isr.h
 * @author Konstantin Tcholokachvili
 * @date 2013
 * Exceptions handling
 */

#ifndef _ISR_H_
#define _ISR_H_

#include <arch/x86-all/registers.h>

void x86_exceptions_setup(void);

void x86_exception_handler(struct regs *r);

#endif
