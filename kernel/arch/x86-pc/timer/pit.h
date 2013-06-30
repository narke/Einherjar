/**
 * @file pit.h
 * @author Konstantin Tcholokachvili
 * @date 2013
 * Programmable Interrupt Timer
 */

#ifndef _PIT_H_
#define _PIT_H_

#include <arch/all/types.h>
#include <arch/x86/registers.h>

/**
 * @file pit.h Programmable Interval Timer
 * @author Konstantin Tcholokachvili
 *
 * @see [en] i82C54's datasheet
 * @see [en] http://www.osdever.net/bkerndev/Docs/pit.htm
 * @see [en] http://www.jamesmolloy.co.uk/tutorial_html/5.-IRQs and the PIT.html
 */

/** 
 * Changes timer interrupt frequency from the default one (18.222 Hz)
 * 
 * @param frequency
 * @return status
 */
uint16_t x86_pit_set_frequency(uint32_t frequency);

/**
* Timer's interrupt handler called periodically
*
* @param id
* @return Nothing
*/
void timer_interrupt_handler(struct regs *r);

#endif

