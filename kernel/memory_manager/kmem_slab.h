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
#ifndef _SOS_KMEM_SLAB_H_
#define _SOS_KMEM_SLAB_H_

/**
 * @file kmem_slab.h
 *
 * Kernel Memory Allocator based on Bonwick's slab llocator (Solaris
 * 2.4, Linux 2.4). This allocator achieves good memory utilization
 * ratio (memory effectively used / memory requested) ie limited
 * fragmentation, while elegantly handling cache-effect considerations
 * (TLB locality through the notion of "cache" of slabs, and the
 * dcache utilization through the notion of cache colouring to
 * decrease the conflicts in the dcache for accesses to different data
 * in the same cache).
 *
 * This allocator relies on the range allocator (kmem_vmm.h) to
 * allocate the slabs, which itself relies on the slab allocator to
 * allocate its "range" data structures, thus leading to a
 * chicken-and-egg problem. We solve this problem by introducing the
 * notion of "min_free_objs" for the slab caches, in order for the cache
 * of ranges to always have enough ranges in reserve to complete the
 * range allocation before being urged to allocate a new slab of
 * ranges, which would require the allocation of a new range.
 *
 * Compared to Bonwick's recommendations, we don't handle ctor/dtor
 * routines on the objects, so that we can alter the objects once they
 * are set free. Thus, the list of free object is stored in the free
 * objects themselves, not alongside the objects (this also implies that
 * the SOS_KSLAB_CREATE_MAP flag below is meaningless). We also don't
 * implement the cache colouring (trivial to add, but we omit it for
 * readability reasons), and the only alignment constraint we respect
 * is that allocated objects are aligned on a 4B boundary: for other
 * alignment constraints, the user must integrate them in the
 * "object_size" parameter to "sos_kmem_cache_create()".
 *
 * References :
 * - J. Bonwick's paper, "The slab allocator: An object-caching kernel
 *   memory allocator", In USENIX Summer 1994 Technical Conference
 * - The bible, aka "Unix internals : the new frontiers" (section
 *   12.10), Uresh Vahalia, Prentice Hall 1996, ISBN 0131019082
 * - "The Linux slab allocator", B. Fitzgibbons,
 *   http://www.cc.gatech.edu/people/home/bradf/cs7001/proj2/
 * - The Kos, http://kos.enix.org/
 */


#include <arch/all/status.h>
#include <arch/all/types.h>

/** Opaque data structure that defines a Cache of slabs */
struct sos_kslab_cache;

/** Opaque data structure that defines a slab. Exported only to
    kmem_vmm.h */
struct sos_kslab;

#include "kmem_vmm.h"


/** The maximum  allowed pages for each slab */
#define MAX_PAGES_PER_SLAB 32 /* 128 kB */


/**
 * Initialize the slab cache of slab caches, and prepare the cache of
 * kmem_range for kmem_vmm.
 *
 * @param kernel_core_base The virtual address of the first byte used
 * by the kernel code/data
 *
 * @param kernel_core_top The virtual address of the first byte after
 * the kernel code/data.
 *
 * @param sizeof_struct_range the size of the objects (aka "struct
 * sos_kmem_vmm_ranges") to be allocated in the cache of ranges
 *
 * @param first_struct_slab_of_caches (output value) the virtual
 * address of the first slab structure that gets allocated for the
 * cache of caches. The function actually manually allocate the first
 * slab of the cache of caches because of a chicken-and-egg thing. The
 * address of the slab is used by the kmem_vmm_setup routine to
 * finalize the allocation of the slab, in order for it to behave like
 * a real slab afterwards.
 *
 * @param first_slab_of_caches_base (output value) the virtual address
 * of the slab associated to the slab structure.
 *
 * @param first_slab_of_caches_nb_pages (output value) the number of
 * (virtual) pages used by the first slab of the cache of caches.
 *
 * @param first_struct_slab_of_ranges (output value) the virtual address
 * of the first slab that gets allocated for the cache of ranges. Same
 * explanation as above.
 *
 * @param first_slab_of_ranges_base (output value) the virtual address
 * of the slab associated to the slab structure.
 *
 * @param first_slab_of_ranges_nb_pages (output value) the number of
 * (virtual) pages used by the first slab of the cache of ranges.
 *
 * @return the cache of kmem_range immediatly usable
 */
struct sos_kslab_cache *
sos_kmem_cache_setup_prepare(vaddr_t kernel_core_base,
			     vaddr_t kernel_core_top,
			     size_t  sizeof_struct_range,
			     /* results */
			     struct sos_kslab **first_struct_slab_of_caches,
			     vaddr_t *first_slab_of_caches_base,
			     unsigned int *first_slab_of_caches_nb_pages,
			     struct sos_kslab **first_struct_slab_of_ranges,
			     vaddr_t *first_slab_of_ranges_base,
			     unsigned int *first_slab_of_ranges_nb_pages);

/**
 * Update the configuration of the cache subsystem once the vmm
 * subsystem has been fully initialized
 */
ret_t
sos_kmem_cache_setup_commit(struct sos_kslab *first_struct_slab_of_caches,
			    struct sos_kmem_range *first_range_of_caches,
			    struct sos_kslab *first_struct_slab_of_ranges,
			    struct sos_kmem_range *first_range_of_ranges);


/*
 * Flags for sos_kmem_cache_create()
 */
/** The slabs should be initially mapped in physical memory */
#define SOS_KSLAB_CREATE_MAP  (1<<0)
/** The object should always be set to zero at allocation (implies
    SOS_KSLAB_CREATE_MAP) */
#define SOS_KSLAB_CREATE_ZERO (1<<1)

/**
 * @note this function MAY block (involved allocations are not atomic)
 * @param name must remain valid during the whole cache's life
 *             (shallow copy) !
 * @param cache_flags An or-ed combination of the SOS_KSLAB_CREATE_* flags
 */
struct sos_kslab_cache *
sos_kmem_cache_create(const char* name,
		      size_t  object_size,
		      uint32_t pages_per_slab,
		      uint32_t min_free_objects,
		      uint32_t  cache_flags);

ret_t sos_kmem_cache_destroy(struct sos_kslab_cache *kslab_cache);


/*
 * Flags for sos_kmem_cache_alloc()
 */
/** Allocation should either succeed or fail, without blocking */
#define SOS_KSLAB_ALLOC_ATOMIC (1<<0)

/**
 * Allocate an object from the given cache.
 *
 * @param alloc_flags An or-ed combination of the SOS_KSLAB_ALLOC_* flags
 */
vaddr_t sos_kmem_cache_alloc(struct sos_kslab_cache *kslab_cache,
				 uint32_t alloc_flags);


/**
 * Free an object (assumed to be already allocated and not already
 * free) at the given virtual address.
 */
ret_t sos_kmem_cache_free(vaddr_t vaddr);


/*
 * Function reserved to kmem_vmm.c. Does almost everything
 * sos_kmem_cache_free() does, except it does not call
 * sos_kmem_vmm_del_range() if it needs to. This is aimed at avoiding
 * large recursion when a range is freed with
 * sos_kmem_vmm_del_range().
 *
 * @param the_range The range structure to free
 *
 * @return NULL when the range containing 'the_range' still contains
 * other ranges, or the address of the range which owned 'the_range'
 * if it becomes empty.
 */
struct sos_kmem_range *
sos_kmem_cache_release_struct_range(struct sos_kmem_range *the_range);


#endif /* _SOS_KMEM_SLAB_H_ */
