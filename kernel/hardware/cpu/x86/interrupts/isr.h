#ifndef _ISR_H_
#define _ISR_H_

#include <hardware/cpu/x86/registers.h>

void x86_exceptions_setup(void);

void x86_exception_handler(struct regs *r);

#endif
