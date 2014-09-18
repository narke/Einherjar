#ifndef _REGISTERS_H_
#define _REGISTERS_H_

/**
 * @struct regs
 * 
 * This defines what the stack looks like after an ISR was running
 */
struct regs
{
    unsigned int gs, fs, es, ds;
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
    unsigned int interrupt_number, error_code;
    unsigned int eip, cs, eflags, user_esp, ss;    
};

#endif // _REGISTERS_H_
