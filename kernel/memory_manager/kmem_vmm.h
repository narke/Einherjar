/* Copyright (C) 2000 Thomas Petazzoni
   Copyright (C) 2004 David Decotigny

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307,
   USA. 
*/
#ifndef _SOS_KMEM_VMM_H_
#define _SOS_KMEM_VMM_H_

/**
 * @file kmem_vmm.h
 *
 * Kernel Memory Allocator for multiple-page-sized objects residing in
 * the kernel (virtual memory) space. Relies on the slab cache
 * allocator to allocate its (internal) "range" data structure.
 */

#include <arch/x86-all/mmu/paging.h>

/* The base and top virtual addresses covered by the kernel allocator */
#define SOS_KMEM_VMM_BASE 0x4000 /* 16kB */
#define SOS_KMEM_VMM_TOP  PAGING_MIRROR_VIRTUAL_ADDRESS /* 1GB - 4MB */

/** Opaque structure used internally and declared here for physmem.h */
struct sos_kmem_range;

#include "kmem_slab.h"

/**
 * Mark the areas belonging to SOS_KMEM_VMM_BASE and SOS_KMEM_VMM_TOP
 * are either used or free. Those that are already mapped are marked
 * as "used", and the 0..SOS_KMEM_VMM_BASE virtual addresses as marked
 * as "used" too (to detect incorrect pointer dereferences).
 */
ret_t sos_kmem_vmm_setup(vaddr_t kernel_core_base,
			     vaddr_t kernel_core_top,
			     vaddr_t bootstrap_stack_bottom_addr,
			     vaddr_t bootstrap_stack_top_addr);


/*
 * Flags for kmem_vmm_new_range and kmem_vmm_alloc
 */
/** Physical pages should be immediately mapped */
#define SOS_KMEM_VMM_MAP    (1<<0)
/** Allocation should either success or fail, without blocking */
#define SOS_KMEM_VMM_ATOMIC (1<<1)

/**
 * Allocate a new kernel area spanning one or multiple pages.
 *
 * @param range_base_vaddr If not NULL, the start address of the range
 * is stored in this location
 * @eturn a new range structure
 */
struct sos_kmem_range *sos_kmem_vmm_new_range(size_t  nb_pages,
					      uint32_t  flags,
					      vaddr_t *range_base_vaddr);
ret_t sos_kmem_vmm_del_range(struct sos_kmem_range *range);


/**
 * Straighforward variant of sos_kmem_vmm_new_range() returning the
 * range's start address instead of the range structure
 */
vaddr_t sos_kmem_vmm_alloc(size_t nb_pages,
			       uint32_t flags);

/**
 * @note you are perfectly allowed to give the address of the
 * kernel image, or the address of the bios area here, it will work:
 * the kernel/bios WILL be "deallocated". But if you really want to do
 * this, well..., do expect some "surprises" ;)
 */
ret_t sos_kmem_vmm_free(vaddr_t vaddr);


/**
 * @return TRUE when vaddr is covered by any (used) kernel range
 */
bool_t sos_kmem_vmm_is_valid_vaddr(vaddr_t vaddr);


/* *****************************
 * Reserved to kmem_slab.c ONLY.
 */
/**
 * Associate the range with the given slab.
 */
ret_t sos_kmem_vmm_set_slab(struct sos_kmem_range *range,
				struct sos_kslab *slab);

/**
 * Retrieve the (used) slab associated with the range covering vaddr.
 *
 * @return NULL if the range is not associated with a KMEM range
 */
struct sos_kslab *sos_kmem_vmm_resolve_slab(vaddr_t vaddr);

#endif /* _SOS_KMEM_VMM_H_ */
