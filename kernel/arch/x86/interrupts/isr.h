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

#include <arch/x86/registers.h>

/** Setup interruption service request handling */
void x86_isr_setup(void);

/** Interruption service request handler */
void x86_isr_handler(struct regs *r);

#endif // _ISR_H_
