/**
 * @author Konstantin Tcholokachvili
 * @date 2013
 * @license MIT License
 *
 * Handling interruptions descriptor table
 */

#include <arch/x86/mmu/segment.h>
#include <lib/status.h>
#include "idt.h"

static struct x86_idt_entry global_idt[INTERRUPTIONS_MAX_LIMIT];

void x86_idt_setup(void)
{
	struct x86_idtr idtr;
	uint16_t i;
	
	for (i = 0; i < INTERRUPTIONS_MAX_LIMIT; i++)
	{
		struct x86_idt_entry *idt_entry = global_idt+i;
		
		idt_entry->segment_selector 	= 0x08;
		idt_entry->unused		= 0;
		
		/* = 0xE for i386 32-bit interrupt gate */
		idt_entry->gate_type		= 0xE;
		
		/* = 0 for interrupt gates */
		idt_entry->storage_segment	= 0;
		
		/* This IDT entry is disabled by default */
		x86_idt_set_handler(i, (uint32_t)NULL);
	}	
	

	idtr.base_address	= (uint32_t)global_idt;
	idtr.limit		= sizeof(global_idt) - 1;

	asm volatile ("lidt %0\n"::"m"(idtr):"memory");
} 


ret_t x86_idt_set_handler(uint32_t index,
			      uint32_t handler_address)
{
	struct x86_idt_entry *idt_entry;

	if (index >= INTERRUPTIONS_MAX_LIMIT)
		return -KERNEL_INVALID_VALUE;
	
	idt_entry = global_idt + index;
  
	if (handler_address != (uint32_t)NULL)
	{
		/* Enabling IDT entry*/
		idt_entry->offset_low  = handler_address & 0xffff;
		idt_entry->offset_high = (handler_address >> 16) & 0xffff;
		idt_entry->descriptor_privilidge_level	= 0;
		idt_entry->present     = 1;	/* Yes, there is a handler */
	}
	else
	{
		/* Disabling IDT entry*/
		idt_entry->offset_low  = 0;
		idt_entry->offset_high = 0;
		idt_entry->descriptor_privilidge_level	= 0;
		idt_entry->present     = 0;	/* No, there is no handler */
	}

	return KERNEL_OK;
}
