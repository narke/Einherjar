#ifndef _IDT_H_
#define _IDT_H_

/**
 * @file idt.h
 * @author Konstantin Tcholokachvili
 * @date 2013
 * @license MIT License
 *
 * @see IA-32 Intel Architecture Software Developer's Manual, Volume 3 [Chapter 3]
 * @note The chosen segmentation mechanism model is Basic Flat Model as Paging allows closer control
 */


#include <lib/types.h>

/** Maximum number of interruptions which can be handled */
#define INTERRUPTIONS_MAX_LIMIT	256


/* Mapping of the IRQ lines in the IDT */
#define X86_IRQ_BASE	32
#define X86_IRQ_NUMBER	16
#define X86_IRQ_MAX	X86_IRQ_BASE + X86_IRQ_NUMBER - 1


/**
 * The IDT register stores the address and size of the IDT.
 *
 * @see Intel x86 doc vol 3, section 2.4, figure 2-4
 */
struct x86_idtr
{
	uint16_t limit;
	uint32_t base_address;
} __attribute__((packed, aligned (8)));

/**
 * An entry in the IDT is a reference to a interrupt/trap routine or 
 * a task gate to handle the software/hardware interrupts and 
 * exceptions.
 *
 * @see figure 5-2, intel x86 doc, vol 3
 */
struct x86_idt_entry
{
	/* Low dword */
	/** 0..15, offset of the routine in the segment */
	uint16_t offset_low;
	/** 16..31, the ID of the segment */
	uint16_t segment_selector;

	/* High dword */
	/** 32..39 */
	uint8_t unused:8;
	/** 40..43 (interrupt gate, trap gate...) */
	uint8_t gate_type:4;   
	/** 44 */
	uint8_t storage_segment:1;
	/** 45,46 */
	uint8_t descriptor_privilidge_level:2;
	/** 47 */
	uint8_t present:1;
	/** 48..63 */ 
	uint16_t offset_high;
} __attribute__((packed));


/** Setup the interrupt descriptor table */
void x86_idt_setup(void);

/**
 * Set an interrupt handler with a callback
 * @param index 		Index in IDT
 * @param handler_address	The callback which will be called
 * @return KERNEL_OK on success or KERNEL_INVALID_VALUE on failure
 * */
ret_t x86_idt_set_handler(uint32_t index,
			      uint32_t handler_address);

#endif // _IDT_H_
