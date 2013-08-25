#ifndef _GDT_H_
#define _GDT_H_

/**
 * @file gdt.h
 * @author Konstantin Tcholokachvili
 * @date 2013
 * @see IA-32 Intel Architecture Software Developer's Manual, Volume 3 [Chapter 3]
 * @note The chosen segmentation mechanism model is Basic Flat Model as
 * paging allows closer control
 *
 * GDT setup
 */

#include <arch/all/types.h>

/** Setup GDT by initializing the GDTR register */
void x86_gdt_setup(void);

/** Setup the kernel TSS to manage ring 3 - ring 0 mode switching */
void x86_gdt_kernel_tss_setup(vaddr_t tss_virtual_address);

#endif // _GDT_H_
