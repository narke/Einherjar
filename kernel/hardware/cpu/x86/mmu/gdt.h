/**
 * @file gdt.h
 * @author Konstantin Tcholokachvili
 * @see IA-32 Intel Architecture Software Developer's Manual, Volume 3 [Chapter 3]
 * @note The chosen segmentation mechanism model is Basic Flat Model as
 * paging allows closer control
 * @date 2013
 */

#include <libraries/types.h>

#ifndef _GDT_H_
#define _GDT_H_


/** Represents a segment descriptor data type */
struct x86_segment_descriptor
{
	/* Lowest dword */
  	/** Segment Limit: bits 15..0 */
  	uint16_t segment_limit_15_0;
  	/** Base Address: bits 15..0  */
  	uint16_t base_paged_address_15_0;

  	/* Highest dword */
  	/** Base Address: bits 23..16 */
  	uint8_t base_paged_address_23_16;
  	/** Segment Type (code/data) */
  	uint8_t segment_type:4;
  	/** 0 = system, 1 = Code/Data  */
  	uint8_t descriptor_type:1;
  	/** Descriptor Privilege Level */
  	uint8_t descriptor_privilege_level:2;
  	/** Segment Present */
  	uint8_t segment_present:1;
  	/** Segment Limit: bits 19..16 */
  	uint8_t segment_limit_19_16:4;
  	/** Available for any use */
  	uint8_t available:1;
  	uint8_t zero:1;
  	/** 0=16 bits instructions, 1=32 bits */
  	uint8_t operand_size:1;
  	/** 0=limit in bytes, 1=limit in pages */
  	uint8_t granularity:1;
  	/** Base address bits 31..24 */
  	uint8_t base_paged_address_31_24;
} __attribute__ ((packed, aligned (8)));


/** Describes the GDTR register */
struct x86_gdtr
{
	/** The limit address represents the maximal offset of the GDTR register */
	uint16_t  limit;
	/** The base (linear, in paged memory) address represents
		the starting address of the GDTR register */
	uint32_t linear_base_address;
} __attribute__((packed, aligned(8)));


/** Setup GDT by initializing the GDTR register */
void x86_gdt_setup(void);


#endif
