/**
 * @file irq.c
 * @author Konstantin Tcholokachvili
 * @date 2013
 * IRQs routines and PIC handling 
 */

#include "irq.h"
#include <arch/x86/io_ports.h>
#include <arch/x86/interrupts/idt.h>
#include <arch/all/types.h>

#define PIC_MASTER_COMMAND 0x20
#define PIC_MASTER_DATA	   0x21
#define PIC_SLAVE_COMMAND  0xA0
#define PIC_SLAVE_DATA	   0xA1

extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();


/* This array is actually an array of function pointers. We use
*  this to handle custom IRQ handlers for a given IRQ */
void *irq_routines[16] =
{
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

/* This installs a custom IRQ handler for the given IRQ */
void x86_irq_set_handler(int irq, void (*handler)(struct regs *r))
{
    irq_routines[irq] = handler;
}


/* This clears the handler for a given IRQ */
void x86_irq_unset_handler(int irq)
{
    irq_routines[irq] = 0;
}

/* Normally, IRQs 0 to 7 are mapped to entries 8 to 15. This
*  Double Fault! Without remapping, every time IRQ0 fires,
*  you get a Double Fault Exception, which is NOT actually
*  what's happening. We send commands to the Programmable
*  Interrupt Controller (PICs - also called the 8259's) in
*  order to make IRQ0 to 15 be remapped to IDT entries 32 to
*  47 */

void pic_setup(void)
{
    /* Send ICW1: 8086 mode + NOT Single ctrl + call address
     interval=8 */
    outb(PIC_MASTER_COMMAND, 0x11);
    outb(PIC_SLAVE_COMMAND,  0x11);

    /* Send ICW2: ctrl base address */
    outb(PIC_MASTER_DATA, 0x20);
    outb(PIC_SLAVE_DATA,  0x28);
	
    /* Send ICW3 master: mask where slaves are connected */
    outb(PIC_MASTER_DATA, 0x04);
    
    /* Send ICW3 slave: index where the slave is connected on master */
    outb(PIC_SLAVE_DATA,  0x02);

    /* Send ICW4: 8086 mode, fully nested, not buffered, no implicit EOI */
    outb(PIC_MASTER_DATA, 0x01);
    outb(PIC_SLAVE_DATA,  0x01);
	
    /* Masking interruptions */
    outb(PIC_MASTER_DATA, 0x0);
    outb(PIC_SLAVE_DATA,  0x0);
}



/* We first remap the interrupt controllers, and then we install
*  the appropriate ISRs to the correct entries in the IDT. This
*  is just like installing the exception handlers */

void x86_irq_setup(void)
{
    pic_setup();

    x86_idt_set_handler(32, (uint32_t)irq0, RING0);
    x86_idt_set_handler(33, (uint32_t)irq1, RING0);
    x86_idt_set_handler(34, (uint32_t)irq2, RING0);
    x86_idt_set_handler(35, (uint32_t)irq3, RING0);
    x86_idt_set_handler(36, (uint32_t)irq4, RING0);
    x86_idt_set_handler(37, (uint32_t)irq5, RING0);
    x86_idt_set_handler(38, (uint32_t)irq6, RING0);
    x86_idt_set_handler(39, (uint32_t)irq7, RING0);

    x86_idt_set_handler(40, (uint32_t)irq8,  RING0);
    x86_idt_set_handler(41, (uint32_t)irq9,  RING0);
    x86_idt_set_handler(42, (uint32_t)irq10, RING0);
    x86_idt_set_handler(43, (uint32_t)irq11, RING0);
    x86_idt_set_handler(44, (uint32_t)irq12, RING0);
    x86_idt_set_handler(45, (uint32_t)irq13, RING0);
    x86_idt_set_handler(46, (uint32_t)irq14, RING0);
    x86_idt_set_handler(47, (uint32_t)irq15, RING0);
}



/* Each of the IRQ ISRs point to this function, rather than
*  the 'fault_handler' in 'isrs.c'. The IRQ Controllers need
*  to be told when you are done servicing them, so you need
*  to send them an "End of Interrupt" command (0x20). There
*  are two 8259 chips: The first exists at 0x20, the second
*  exists at 0xA0. If the second controller (an IRQ from 8 to
*  15) gets an interrupt, you need to acknowledge the
*  interrupt at BOTH controllers, otherwise, you only send
*  an EOI command to the first controller. If you don't send
*  an EOI, you won't raise any more IRQs */

void irq_handler(struct regs *r)
{
    /* This is a blank function pointer */
    void (*handler)(struct regs *r);

    /* Find out if we have a custom handler to run for this
    *  IRQ, and then finally, run it */
    handler = irq_routines[r->interrupt_number - 32];

    if (handler)
    {
        handler(r);
    }



    /* If the IDT entry that was invoked was greater than 40
    *  (meaning IRQ8 - 15), then we need to send an EOI to
    *  the slave controller */
    if (r->interrupt_number >= 40)
    {
        outb(PIC_SLAVE_COMMAND, PIC_MASTER_COMMAND);
    }


    /* In either case, we need to send an EOI to the master
    *  interrupt controller too */
    outb(PIC_MASTER_COMMAND, PIC_MASTER_COMMAND);
}


