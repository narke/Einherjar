#ifndef _IRQ_H_
#define _IRQ_H_

/**
 * @file irq.h
 * @author Konstantin Tcholokachvili
 * @date 2013
 * @license MIT License
 * 
 * IRQs constants and functions
 */


#include <arch/x86-all/registers.h>

#define IRQ_TIMER	  0
#define IRQ_KEYBOARD	  1
#define IRQ_SLAVE_PIC     2
#define IRQ_COM2          3
#define IRQ_COM1          4
#define IRQ_LPT2          5
#define IRQ_FLOPPY        6
#define IRQ_LPT1          7
#define IRQ_8_NOT_DEFINED 8
#define IRQ_RESERVED_1    9
#define IRQ_RESERVED_2    10
#define IRQ_RESERVED_3    11
#define IRQ_RESERVED_4    12
#define IRQ_COPROCESSOR   13
#define IRQ_HARDDISK      14
#define IRQ_RESERVED_5    15


#define X86_IRQs_DISABLE(flags) \
    ({asm volatile("pushfl ; popl %0":"=g"(flags)::"memory"); asm("cli\n");})

#define X86_IRQs_ENABLE(flags) \
    asm volatile("push %0; popfl"::"g"(flags):"memory");


/** Set an IRQ handler function
 *
 * @param irq IRQ number
 * @param handler A callback function which will be called when the IRQ is raised
 */
void x86_irq_set_handler(int irq, void (*handler)(struct regs *r));

/** Unset an IRQ handler function
 * 
 * @param irq IRQ number
 */
void x86_irq_unset_handler(int irq);

/** Setup IRQs handling */
void x86_irq_setup(void);

#endif // _IRQ_H_
