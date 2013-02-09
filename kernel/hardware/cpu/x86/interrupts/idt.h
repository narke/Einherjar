/**
 * @file idt.h
 * @author Konstantin Tcholokachvili
 * @see IA-32 Intel Architecture Software Developer's Manual, Volume 3 [Chapter 3]
 * @note The chosen segmentation mechanism model is Basic Flat Model as
 * paging allows closer control
 * @date 2013
 */

#ifndef _IDT_H_
#define _IDT_H_

#include <libraries/types.h>

/** Maximum number of interruptions which can be handled */
#define INTERRUPTIONS_MAX_LIMIT	256


/* Mapping of the IRQ lines in the IDT */
#define X86_IRQ_BASE	32
#define X86_IRQ_NUMBER	16
#define X86_IRQ_MAX	X86_IRQ_BASE + X86_IRQ_NUMBER - 1)

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
	/** 15..0, offset of the routine in the segment */
	uint16_t offset_low;
	/** 31..16, the ID of the segment */
	uint16_t segment_selector;

	/* High dword */
	/** 4..0 */
	uint8_t reserved:5;
	/** 7..5 */
	uint8_t flags:3;
	/** 10..8 (interrupt gate, trap gate...) */
	uint8_t type:3;   
	/** 11 (0=16bits instructions, 1=32bits instructions) */    
	uint8_t operation_size:1;
	/** 12 */
	uint8_t zero:1;
	/** 14..13 */
	uint8_t descriptor_privilidge_level:2;
	/** 15 */
	uint8_t present:1;
	/** 31..16 */ 
	uint16_t offset_high;
} __attribute__((packed));


/** Setup the interrupt descriptor table */
void x86_idt_setup(void);

/**
 * Set an interrupt handler with a callback
 * @param index 			Index in IDT
 * @param handler_address	The callback which will be called
 * @param lowest_priviledge	The range value must be 0..3
 * @return KERNEL_OK on success or KERNEL_INVALID_VALUE on failure
 * */
uint16_t x86_idt_set_handler(uint32_t index,
			      uint32_t handler_address,
			      uint32_t lowest_priviledge);

#endif 
