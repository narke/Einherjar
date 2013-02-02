/**
 * @file gdt.c
 * @author Konstantin Tcholokachvili
 * @date 2013
 */

#include "gdt.h"
#include "segment.h"


/**
 * Helper macro that builds a Segment descriptor for the virtual
 * 0..4GB addresses to be mapped to the 0..4GB linear addresses.
 */
#define BUILD_GDT_ENTRY(descr_privilege_level, is_code)				   \
  ((struct x86_segment_descriptor) {					               \
      .segment_limit_15_0			= 0xffff,			               \
      .base_paged_address_15_0		= 0,                               \
      .base_paged_address_23_16		= 0,                               \
      .segment_type					= ((is_code)?0xb:0x3),             \
      .descriptor_type				= 1,  /* 1=Code/Data */            \
      .descriptor_privilege_level	= ((descr_privilege_level) & 0x3), \
      .segment_present				= 1,                               \
      .segment_limit_19_16			= 0xf,                             \
      .available					= 0,                               \
      .operand_size					= 1,  /* 32 bits instr/data */     \
      .granularity					= 1   /* limit is in 4kB Pages */  \
  })


static struct x86_segment_descriptor gdt[] = {
	[NULL_SEGMENT]  = (struct x86_segment_descriptor){ 0, },
	[KERNEL_CODE_SEGMENT] = BUILD_GDT_ENTRY(0, 1),
	[KERNEL_DATA_SEGMENT] = BUILD_GDT_ENTRY(0, 0),
	[USER_CODE_SEGMENT]   = BUILD_GDT_ENTRY(3, 1),
	[USER_DATA_SEGMENT]   = BUILD_GDT_ENTRY(3, 0),
	[KERNEL_TSS_SEGMENT]  = { 0, }
};

void x86_gdt_setup(void)
{
	struct x86_gdtr gdtr;

	/* GDT's starting address */
	gdtr.linear_base_address = (uint32_t)gdt;
	/* GDT's upper limit address */
	gdtr.limit = sizeof(gdt) - 1;

    /*
    	Setup CPU's GDT and update the segment registers.
    	The CS register may only be updated with a long jump
    	to an absolute address in the given segment
    	(Intel x86 manual, vol 3, section 4.8.1)
    */
	__asm__ __volatile__ ("lgdt %0	\n\
                 ljmp %1,$1f      \n\
                 1:               \n\
                 movw %2,    %%ax \n\
                 movw %%ax,  %%ss \n\
                 movw %%ax,  %%ds \n\
                 movw %%ax,  %%es \n\
                 movw %%ax,  %%fs \n\
                 movw %%ax,  %%gs"
		:
		:"m"(gdtr),
		 "i"(X86_BUILD_SEGMENT_REGISTER_VALUE(0, FALSE, KERNEL_CODE_SEGMENT)),
		 "i"(X86_BUILD_SEGMENT_REGISTER_VALUE(0, FALSE, KERNEL_DATA_SEGMENT))
		:"memory","eax");
}
