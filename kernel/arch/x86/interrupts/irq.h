/**
 * @file irq.h
 * @author Konstantin Tcholokachvili
 * @date 2013
 * IRQs constants and functions
 */

#ifndef _IRQ_H_
#define _IRQ_H_

#include <arch/x86/registers.h>

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

extern void x86_irq_set_handler(int irq, void (*handler)(struct regs *r));
extern void x86_irq_unset_handler(int irq);
extern void x86_irq_setup(void);

#endif
