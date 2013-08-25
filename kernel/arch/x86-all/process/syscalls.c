#include <arch/x86-all/interrupts/idt.h>
#include <arch/x86-all/interrupts/irq.h>
#include <arch/x86-all/mmu/segment.h>
#include <arch/x86-pc/input_output/screen/vga.h>

#include "syscalls.h"

DEFN_SYSCALL1(vga_display_string, 0, const char*);

uint32_t nb_syscalls;

static void *syscalls[] =
{
	&vga_display_string,
	0
};


typedef uint32_t (*syscall_function)(unsigned int, ...);


void syscall_handler(struct regs *r)
{
	uint32_t retval;

	if (r->eax >= nb_syscalls)
		return;

	void *location = syscalls[r->eax];

	syscall_function function = (syscall_function)location;
	
	retval = function(r->ebx, r->ecx, r->edx, r->esi, r->edi);
	
	r->eax = retval;
}

extern void isr128();

void syscalls_setup()
{
	uint32_t flags;

	X86_IRQs_DISABLE(flags);

	for (nb_syscalls = 0; syscalls[nb_syscalls]; ++nb_syscalls)
		;

	x86_idt_set_handler(SYSCALL_ID, (uint32_t)isr128, RING3);
	x86_irq_set_handler(SYSCALL_ID, syscall_handler);

	X86_IRQs_ENABLE(flags);
}
