/**
 * @author Konstantin Tcholokachvili
 * @date 2013
 * @license MIT License
 * 
 * Exceptions handling
 */

#include <arch/x86/interrupts/idt.h>
#include <arch/x86/mmu/segment.h>
#include <lib/types.h>
#include <lib/libc.h>

#include "isr.h"

#define EXCEPTIONS_NUMBER 32

extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();


void x86_isr_setup(void)
{
    x86_idt_set_handler(0,  (uint32_t)isr0);
    x86_idt_set_handler(1,  (uint32_t)isr1);
    x86_idt_set_handler(2,  (uint32_t)isr2);
    x86_idt_set_handler(3,  (uint32_t)isr3);
    x86_idt_set_handler(4,  (uint32_t)isr4);
    x86_idt_set_handler(5,  (uint32_t)isr5);
    x86_idt_set_handler(6,  (uint32_t)isr6);
    x86_idt_set_handler(7,  (uint32_t)isr7);
    x86_idt_set_handler(8,  (uint32_t)isr8);
    x86_idt_set_handler(9,  (uint32_t)isr9);
    x86_idt_set_handler(10, (uint32_t)isr10);
    x86_idt_set_handler(11, (uint32_t)isr11);
    x86_idt_set_handler(12, (uint32_t)isr12);
    x86_idt_set_handler(13, (uint32_t)isr13);
    x86_idt_set_handler(14, (uint32_t)isr14);
    x86_idt_set_handler(15, (uint32_t)isr15);
    x86_idt_set_handler(16, (uint32_t)isr16);
    x86_idt_set_handler(17, (uint32_t)isr17);
    x86_idt_set_handler(18, (uint32_t)isr18);
    x86_idt_set_handler(19, (uint32_t)isr19);
    x86_idt_set_handler(20, (uint32_t)isr20);
    x86_idt_set_handler(21, (uint32_t)isr21);
    x86_idt_set_handler(22, (uint32_t)isr22);
    x86_idt_set_handler(23, (uint32_t)isr23);
    x86_idt_set_handler(24, (uint32_t)isr24);
    x86_idt_set_handler(25, (uint32_t)isr25);
    x86_idt_set_handler(26, (uint32_t)isr26);
    x86_idt_set_handler(27, (uint32_t)isr27);
    x86_idt_set_handler(28, (uint32_t)isr28);
    x86_idt_set_handler(29, (uint32_t)isr29);
    x86_idt_set_handler(30, (uint32_t)isr30);
    x86_idt_set_handler(31, (uint32_t)isr31);
}



/* List of IA-32 exceptions */
char *exception_messages[] =
{
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",
    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved"
};



/* All of our Exception handling Interrupt Service Routines will
*  point to this function. This will tell us what exception has
*  happened! Right now, we simply halt the system by hitting an
*  endless loop. All ISRs disable interrupts while they are being
*  serviced as a 'locking' mechanism to prevent an IRQ from
*  happening and messing up kernel data structures */

void x86_isr_handler(struct regs *r)
{
    if (r->interrupt_number < EXCEPTIONS_NUMBER)
    {
        printf(">> Exception: %s. System Halted! <<\n", exception_messages[r->interrupt_number]);
        for (;;);
    }
}


