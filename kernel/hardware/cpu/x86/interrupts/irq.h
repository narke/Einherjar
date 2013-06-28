#ifndef _IRQ_H_
#define _IRQ_H_

#include <hardware/cpu/x86/registers.h>

extern void x86_irq_set_handler(int irq, void (*handler)(struct regs *r));
extern void x86_irq_unset_handler(int irq);
extern void x86_irq_setup(void);

#endif
