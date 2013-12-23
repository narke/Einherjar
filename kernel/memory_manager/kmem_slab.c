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

#include <arch/all/klibc.h>
#include <arch/all/queue.h>
#include <arch/x86-all/mmu/paging.h>
#include <memory_manager/physical_memory.h>

#include "kmem_vmm.h"
#include "kmem_slab.h"

/* Dimensioning constants */
#define NB_PAGES_IN_SLAB_OF_CACHES 1
#define NB_PAGES_IN_SLAB_OF_RANGES 1

/** The structure of a slab cache */
struct sos_kslab_cache
{
  	char *name;

  	/* non mutable characteristics of this slab */
  	size_t  original_obj_size; /* asked object size */
  	size_t  alloc_obj_size;    /* actual object size, taking the
				    alignment constraints into account */
  	uint32_t nb_objects_per_slab;
  	uint32_t nb_pages_per_slab;
  	uint32_t min_free_objects;

	/* slab cache flags */
	// #define SOS_KSLAB_CREATE_MAP  (1<<0) /* See kmem_slab.h */
	// #define SOS_KSLAB_CREATE_ZERO (1<<1) /* " " " " " " " " */
#define ON_SLAB (1<<31) /* struct sos_kslab is included inside the slab */
	uint32_t  flags;

	/* Supervision data (updated at run-time) */
	uint32_t nb_free_objects;

	/* The lists of slabs owned by this cache */
	struct sos_kslab *slab_list; /* head = non full, tail = full */

  	/* The caches are linked together on the kslab_cache_list */
	CIRCLEQ_ENTRY(sos_kslab_cache) slab_cache_entries;
};


/** The structure of a slab */
struct sos_kslab
{
  /** Number of free objects on this slab */
  uint32_t nb_free;

  /** The list of these free objects */
  struct sos_kslab_free_object *free;

  /** The address of the associated range structure */
  struct sos_kmem_range *range;

  /** Virtual start address of this range */
  vaddr_t first_object;
  
  /** Slab cache owning this slab */
  struct sos_kslab_cache *cache;

  /** Links to the other slabs managed by the same cache */
  CIRCLEQ_ENTRY(sos_kslab) slab_entries;
};


/** The structure of the free objects in the slab */
struct sos_kslab_free_object
{
  CIRCLEQ_ENTRY(sos_kslab_free_object) free_objects;
};

/** The cache of slab caches */
static struct sos_kslab_cache *cache_of_struct_kslab_cache;

/** The cache of slab structures for non-ON_SLAB caches */
static struct sos_kslab_cache *cache_of_struct_kslab;

/** The list of slab caches */
static struct sos_kslab_cache *kslab_cache_list;

/* Helper function to initialize a cache structure */
static ret_t
cache_initialize(/*out*/struct sos_kslab_cache *the_cache,
		 const char* name,
		 size_t  obj_size,
		 uint32_t pages_per_slab,
		 uint32_t min_free_objs,
		 uint32_t  cache_flags)
{
  unsigned int space_left;
  size_t alloc_obj_size;

  if (obj_size <= 0)
    return -KERNEL_INVALID_VALUE;

  /* Default allocation size is the requested one */
  alloc_obj_size = obj_size;

  /* Make sure the requested size is large enough to store a
     free_object structure */
  if (alloc_obj_size < sizeof(struct sos_kslab_free_object))
    alloc_obj_size = sizeof(struct sos_kslab_free_object);
  
  /* Align obj_size on 4 bytes */
  alloc_obj_size = __PAGE_ALIGN_UPPER(alloc_obj_size, sizeof(int));

  /* Make sure supplied number of pages per slab is consistent with
     actual allocated object size */
  if (alloc_obj_size > pages_per_slab*X86_PAGE_SIZE)
    return -KERNEL_INVALID_VALUE;
  
  /* Refuse too large slabs */
  if (pages_per_slab > MAX_PAGES_PER_SLAB)
    return -KERNEL_NO_MEMORY;

  /* Fills in the cache structure */
  memset(the_cache, 0x0, sizeof(struct sos_kslab_cache));
  the_cache->name              = (char*)name;
  the_cache->flags             = cache_flags;
  the_cache->original_obj_size = obj_size;
  the_cache->alloc_obj_size    = alloc_obj_size;
  the_cache->min_free_objects  = min_free_objs;
  the_cache->nb_pages_per_slab = pages_per_slab;
  
  /* Small size objets => the slab structure is allocated directly in
     the slab */
  if(alloc_obj_size <= sizeof(struct sos_kslab))
    the_cache->flags |= ON_SLAB;
  
  /*
   * Compute the space left once the maximum number of objects
   * have been allocated in the slab
   */
  space_left = the_cache->nb_pages_per_slab*X86_PAGE_SIZE;
  if(the_cache->flags & ON_SLAB)
    space_left -= sizeof(struct sos_kslab);
  the_cache->nb_objects_per_slab = space_left / alloc_obj_size;
  space_left -= the_cache->nb_objects_per_slab*alloc_obj_size;

  /* Make sure a single slab is large enough to contain the minimum
     number of objects requested */
  if (the_cache->nb_objects_per_slab < min_free_objs)
    return -KERNEL_INVALID_VALUE;

  /* If there is now enough place for both the objects and the slab
     structure, then make the slab structure ON_SLAB */
  if (space_left >= sizeof(struct sos_kslab))
    the_cache->flags |= ON_SLAB;

  return KERNEL_OK;
}


/** Helper function to add a new slab for the given cache. */
static ret_t
cache_add_slab(struct sos_kslab_cache *kslab_cache,
	       vaddr_t vaddr_slab,
	       struct sos_kslab *slab)
{
  int i;

  /* Setup the slab structure */
  memset(slab, 0x0, sizeof(struct sos_kslab));
  slab->cache = kslab_cache;

  /* Establish the address of the first free object */
  slab->first_object = vaddr_slab;

  /* Account for this new slab in the cache */
  slab->nb_free = kslab_cache->nb_objects_per_slab;
  kslab_cache->nb_free_objects += slab->nb_free;

  /* Build the list of free objects */
  for (i = 0 ; i <  kslab_cache->nb_objects_per_slab ; i++)
    {
      vaddr_t obj_vaddr;

      /* Set object's address */
      obj_vaddr = slab->first_object + i*kslab_cache->alloc_obj_size;

      /* Add it to the list of free objects */
      list_add_tail(slab->free,
		    (struct sos_kslab_free_object *)obj_vaddr);
    }

  /* Add the slab to the cache's slab list: add the head of the list
     since this slab is non full */
  list_add_head(kslab_cache->slab_list, slab);

  return KERNEL_OK;
}


/** Helper function to allocate a new slab for the given kslab_cache */
static ret_t
cache_grow(struct sos_kslab_cache *kslab_cache,
	   uint32_t alloc_flags)
{
  uint32_t range_alloc_flags;

  struct sos_kmem_range *new_range;
  vaddr_t new_range_start;

  struct sos_kslab *new_slab;

  /*
   * Setup the flags for the range allocation
   */
  range_alloc_flags = 0;

  /* Atomic ? */
  if (alloc_flags & SOS_KSLAB_ALLOC_ATOMIC)
    range_alloc_flags |= SOS_KMEM_VMM_ATOMIC;

  /* Need physical mapping NOW ? */
  if (kslab_cache->flags & (SOS_KSLAB_CREATE_MAP
			   | SOS_KSLAB_CREATE_ZERO))
    range_alloc_flags |= SOS_KMEM_VMM_MAP;

  /* Allocate the range */
  new_range = sos_kmem_vmm_new_range(kslab_cache->nb_pages_per_slab,
				     range_alloc_flags,
				     & new_range_start);
  if (! new_range)
    return -KERNEL_NO_MEMORY;

  /* Allocate the slab structure */
  if (kslab_cache->flags & ON_SLAB)
    {
      /* Slab structure is ON the slab: simply set its address to the
	 end of the range */
      vaddr_t slab_vaddr
	= new_range_start + kslab_cache->nb_pages_per_slab*X86_PAGE_SIZE
	  - sizeof(struct sos_kslab);
      new_slab = (struct sos_kslab*)slab_vaddr;
    }
  else
    {
      /* Slab structure is OFF the slab: allocate it from the cache of
	 slab structures */
      vaddr_t slab_vaddr
	= sos_kmem_cache_alloc(cache_of_struct_kslab,
			       alloc_flags);
      if (! slab_vaddr)
	{
	  sos_kmem_vmm_del_range(new_range);
	  return -KERNEL_NO_MEMORY;
	}
      new_slab = (struct sos_kslab*)slab_vaddr;
    }

  cache_add_slab(kslab_cache, new_range_start, new_slab);
  new_slab->range = new_range;

  /* Set the backlink from range to this slab */
  sos_kmem_vmm_set_slab(new_range, new_slab);

  return KERNEL_OK;
}


/**
 * Helper function to release a slab
 *
 * The corresponding range is always deleted, except when the @param
 * must_del_range_now is not set. This happens only when the function
 * gets called from sos_kmem_cache_release_struct_range(), to avoid
 * large recursions.
 */
static ret_t
cache_release_slab(struct sos_kslab *slab,
		   sos_bool_t must_del_range_now)
{
  struct sos_kslab_cache *kslab_cache = slab->cache;
  struct sos_kmem_range *range = slab->range;

  assert(kslab_cache != NULL);
  assert(range != NULL);
  assert(slab->nb_free == slab->cache->nb_objects_per_slab);

  /* First, remove the slab from the slabs' list of the cache */
  list_delete(kslab_cache->slab_list, slab);
  slab->cache->nb_free_objects -= slab->nb_free;

  /* Release the slab structure if it is OFF slab */
  if (! (slab->cache->flags & ON_SLAB))
    sos_kmem_cache_free((vaddr_t)slab);

  /* Ok, the range is not bound to any slab anymore */
  sos_kmem_vmm_set_slab(range, NULL);

  /* Always delete the range now, unless we are told not to do so (see
     sos_kmem_cache_release_struct_range() below) */
  if (must_del_range_now)
    return sos_kmem_vmm_del_range(range);

  return KERNEL_OK;
}


/**
 * Helper function to create the initial cache of caches, with a very
 * first slab in it, so that new cache structures can be simply allocated.
 * @return the cache structure for the cache of caches
 */
static struct sos_kslab_cache *
create_cache_of_caches(vaddr_t vaddr_first_slab_of_caches,
		       int nb_pages)
{
  /* The preliminary cache structure we need in order to allocate the
     first slab in the cache of caches (allocated on the stack !) */
  struct sos_kslab_cache fake_cache_of_caches;

  /* The real cache structure for the cache of caches */
  struct sos_kslab_cache *real_cache_of_caches;

  /* The kslab structure for this very first slab */
  struct sos_kslab       *slab_of_caches;

  /* Init the cache structure for the cache of caches */
  if (cache_initialize(& fake_cache_of_caches,
		       "Caches", sizeof(struct sos_kslab_cache),
		       nb_pages, 0, SOS_KSLAB_CREATE_MAP | ON_SLAB))
    /* Something wrong with the parameters */
    return NULL;

  memset((void*)vaddr_first_slab_of_caches, 0x0, nb_pages*X86_PAGE_SIZE);

  /* Add the pages for the 1st slab of caches */
  slab_of_caches = (struct sos_kslab*)(vaddr_first_slab_of_caches
				       + nb_pages*X86_PAGE_SIZE
				       - sizeof(struct sos_kslab));

  /* Add the abovementioned 1st slab to the cache of caches */
  cache_add_slab(& fake_cache_of_caches,
		 vaddr_first_slab_of_caches,
		 slab_of_caches);

  /* Now we allocate a cache structure, which will be the real cache
     of caches, ie a cache structure allocated INSIDE the cache of
     caches, not inside the stack */
  real_cache_of_caches
    = (struct sos_kslab_cache*) sos_kmem_cache_alloc(& fake_cache_of_caches,
						     0);
  /* We initialize it */
  memcpy(real_cache_of_caches, & fake_cache_of_caches,
	 sizeof(struct sos_kslab_cache));
  /* We need to update the slab's 'cache' field */
  slab_of_caches->cache = real_cache_of_caches;
  
  /* Add the cache to the list of slab caches */
  list_add_tail(kslab_cache_list, real_cache_of_caches);

  return real_cache_of_caches;
}


/**
 * Helper function to create the initial cache of ranges, with a very
 * first slab in it, so that new kmem_range structures can be simply
 * allocated.
 * @return the cache of kmem_range
 */
static struct sos_kslab_cache *
create_cache_of_ranges(vaddr_t vaddr_first_slab_of_ranges,
		       size_t  sizeof_struct_range,
		       int nb_pages)
{
  /* The cache structure for the cache of kmem_range */
  struct sos_kslab_cache *cache_of_ranges;

  /* The kslab structure for the very first slab of ranges */
  struct sos_kslab *slab_of_ranges;

  cache_of_ranges = (struct sos_kslab_cache*)
    sos_kmem_cache_alloc(cache_of_struct_kslab_cache,
			 0);
  if (! cache_of_ranges)
    return NULL;

  /* Init the cache structure for the cache of ranges with min objects
     per slab = 2 !!! */
  if (cache_initialize(cache_of_ranges,
		       "struct kmem_range",
		       sizeof_struct_range,
		       nb_pages, 2, SOS_KSLAB_CREATE_MAP | ON_SLAB))
    /* Something wrong with the parameters */
    return NULL;

  /* Add the cache to the list of slab caches */
  list_add_tail(kslab_cache_list, cache_of_ranges);

  /*
   * Add the first slab for this cache
   */
  memset((void*)vaddr_first_slab_of_ranges, 0x0, nb_pages*X86_PAGE_SIZE);

  /* Add the pages for the 1st slab of ranges */
  slab_of_ranges = (struct sos_kslab*)(vaddr_first_slab_of_ranges
				       + nb_pages*X86_PAGE_SIZE
				       - sizeof(struct sos_kslab));

  cache_add_slab(cache_of_ranges,
		 vaddr_first_slab_of_ranges,
		 slab_of_ranges);

  return cache_of_ranges;
}


struct sos_kslab_cache *
sos_kmem_cache_setup_prepare(vaddr_t kernel_core_base,
			     vaddr_t kernel_core_top,
			     size_t  sizeof_struct_range,
			     /* results */
			     struct sos_kslab **first_struct_slab_of_caches,
			     vaddr_t  *first_slab_of_caches_base,
			     unsigned int *first_slab_of_caches_nb_pages,
			     struct sos_kslab **first_struct_slab_of_ranges,
			     vaddr_t  *first_slab_of_ranges_base,
			     unsigned int *first_slab_of_ranges_nb_pages)
{
  int i;
  ret_t   retval;
  vaddr_t vaddr;

  /* The cache of ranges we are about to allocate */
  struct sos_kslab_cache *cache_of_ranges;

  /* In the begining, there isn't any cache */
  kslab_cache_list = NULL;
  cache_of_struct_kslab = NULL;
  cache_of_struct_kslab_cache = NULL;

  /*
   * Create the cache of caches, initialised with 1 allocated slab
   */

  /* Allocate the pages needed for the 1st slab of caches, and map them
     in kernel space, right after the kernel */
  *first_slab_of_caches_base = SOS_PAGE_ALIGN_SUP(kernel_core_top);
  for (i = 0, vaddr = *first_slab_of_caches_base ;
       i < NB_PAGES_IN_SLAB_OF_CACHES ;
       i++, vaddr += X86_PAGE_SIZE)
    {
      paddr_t ppage_paddr;

      ppage_paddr
	= sos_physmem_ref_physpage_new(FALSE);
      assert(ppage_paddr != (paddr_t)NULL);

      retval = x86_paging_map(ppage_paddr, vaddr,
			      FALSE,
			      VM_FLAG_ATOMIC
			      | VM_FLAG_READ
			      | VM_FLAG_WRITE);
      assert(retval == KERNEL_OK);

      retval = sos_physmem_unref_physpage(ppage_paddr);
      assert(retval == FALSE);
    }

  /* Create the cache of caches */
  *first_slab_of_caches_nb_pages = NB_PAGES_IN_SLAB_OF_CACHES;
  cache_of_struct_kslab_cache
    = create_cache_of_caches(*first_slab_of_caches_base,
			     NB_PAGES_IN_SLAB_OF_CACHES);
  assert(cache_of_struct_kslab_cache != NULL);

  /* Retrieve the slab that should have been allocated */
  *first_struct_slab_of_caches
    = list_get_head(cache_of_struct_kslab_cache->slab_list);

  
  /*
   * Create the cache of ranges, initialised with 1 allocated slab
   */
  *first_slab_of_ranges_base = vaddr;
  /* Allocate the 1st slab */
  for (i = 0, vaddr = *first_slab_of_ranges_base ;
       i < NB_PAGES_IN_SLAB_OF_RANGES ;
       i++, vaddr += X86_PAGE_SIZE)
    {
      paddr_t ppage_paddr;

      ppage_paddr
	= sos_physmem_ref_physpage_new(FALSE);
      assert(ppage_paddr != (paddr_t)NULL);

      retval = x86_paging_map(ppage_paddr, vaddr,
			      FALSE,
			      VM_FLAG_ATOMIC
			      | VM_FLAG_READ
			      | VM_FLAG_WRITE);
      assert(retval == KERNEL_OK);

      retval = sos_physmem_unref_physpage(ppage_paddr);
      assert(retval == FALSE);
    }

  /* Create the cache of ranges */
  *first_slab_of_ranges_nb_pages = NB_PAGES_IN_SLAB_OF_RANGES;
  cache_of_ranges = create_cache_of_ranges(*first_slab_of_ranges_base,
					   sizeof_struct_range,
					   NB_PAGES_IN_SLAB_OF_RANGES);
  assert(cache_of_ranges != NULL);

  /* Retrieve the slab that should have been allocated */
  *first_struct_slab_of_ranges
    = list_get_head(cache_of_ranges->slab_list);

  /*
   * Create the cache of slabs, without any allocated slab yet
   */
  cache_of_struct_kslab
    = sos_kmem_cache_create("off-slab slab structures",
			    sizeof(struct sos_kslab),
			    1,
			    0,
			    SOS_KSLAB_CREATE_MAP);
  assert(cache_of_struct_kslab != NULL);

  return cache_of_ranges;
}


ret_t
sos_kmem_cache_setup_commit(struct sos_kslab *first_struct_slab_of_caches,
			    struct sos_kmem_range *first_range_of_caches,
			    struct sos_kslab *first_struct_slab_of_ranges,
			    struct sos_kmem_range *first_range_of_ranges)
{
  first_struct_slab_of_caches->range = first_range_of_caches;
  first_struct_slab_of_ranges->range = first_range_of_ranges;
  return KERNEL_OK;
}


struct sos_kslab_cache *
sos_kmem_cache_create(const char* name,
		      size_t  obj_size,
		      uint32_t pages_per_slab,
		      uint32_t min_free_objs,
		      uint32_t  cache_flags)
{
  struct sos_kslab_cache *new_cache;

  /* Allocate the new cache */
  new_cache = (struct sos_kslab_cache*)
    sos_kmem_cache_alloc(cache_of_struct_kslab_cache,
			 0/* NOT ATOMIC */);
  if (! new_cache)
    return NULL;

  if (cache_initialize(new_cache, name, obj_size,
		       pages_per_slab, min_free_objs,
		       cache_flags))
    {
      /* Something was wrong */
      sos_kmem_cache_free((vaddr_t)new_cache);
      return NULL;
    }

  /* Add the cache to the list of slab caches */
  list_add_tail(kslab_cache_list, new_cache);
  
  /* if the min_free_objs is set, pre-allocate a slab */
  if (min_free_objs)
    {
      if (cache_grow(new_cache, 0 /* Not atomic */) != KERNEL_OK)
	{
	  sos_kmem_cache_destroy(new_cache);
	  return NULL; /* Not enough memory */
	}
    }

  return new_cache;  
}

  
ret_t sos_kmem_cache_destroy(struct sos_kslab_cache *kslab_cache)
{
  int nb_slabs;
  struct sos_kslab *slab;

  if (! kslab_cache)
    return -KERNEL_INVALID_VALUE;

  /* Refuse to destroy the cache if there are any objects still
     allocated */
  list_foreach(kslab_cache->slab_list, slab, nb_slabs)
    {
      if (slab->nb_free != kslab_cache->nb_objects_per_slab)
	return -KERNEL_BUSY;
    }

  /* Remove all the slabs */
  while ((slab = list_get_head(kslab_cache->slab_list)) != NULL)
    {
      cache_release_slab(slab, TRUE);
    }

  /* Remove the cache */
  return sos_kmem_cache_free((vaddr_t)kslab_cache);
}


vaddr_t sos_kmem_cache_alloc(struct sos_kslab_cache *kslab_cache,
				 uint32_t alloc_flags)
{
  vaddr_t obj_vaddr;
  struct sos_kslab * slab_head;
#define ALLOC_RET return

  /* If the slab at the head of the slabs' list has no free object,
     then the other slabs don't either => need to allocate a new
     slab */
  if ((! kslab_cache->slab_list)
      || (! list_get_head(kslab_cache->slab_list)->free))
    {
      if (cache_grow(kslab_cache, alloc_flags) != KERNEL_OK)
	/* Not enough memory or blocking alloc */
	ALLOC_RET( (vaddr_t)NULL);
    }

  /* Here: we are sure that list_get_head(kslab_cache->slab_list)
     exists *AND* that list_get_head(kslab_cache->slab_list)->free is
     NOT NULL */
  slab_head = list_get_head(kslab_cache->slab_list);
  assert(slab_head != NULL);

  /* Allocate the object at the head of the slab at the head of the
     slabs' list */
  obj_vaddr = (vaddr_t)list_pop_head(slab_head->free);
  slab_head->nb_free --;
  kslab_cache->nb_free_objects --;

  /* If needed, reset object's contents */
  if (kslab_cache->flags & SOS_KSLAB_CREATE_ZERO)
    memset((void*)obj_vaddr, 0x0, kslab_cache->alloc_obj_size);

  /* Slab is now full ? */
  if (slab_head->free == NULL)
    {
      /* Transfer it at the tail of the slabs' list */
      struct sos_kslab *slab;
      slab = list_pop_head(kslab_cache->slab_list);
      list_add_tail(kslab_cache->slab_list, slab);
    }
  
  /*
   * For caches that require a minimum amount of free objects left,
   * allocate a slab if needed.
   *
   * Notice the "== min_objects - 1": we did not write " <
   * min_objects" because for the cache of kmem structure, this would
   * lead to an chicken-and-egg problem, since cache_grow below would
   * call cache_alloc again for the kmem_vmm cache, so we return here
   * with the same cache. If the test were " < min_objects", then we
   * would call cache_grow again for the kmem_vmm cache again and
   * again... until we reach the bottom of our stack (infinite
   * recursion). By telling precisely "==", then the cache_grow would
   * only be called the first time.
   */
  if ((kslab_cache->min_free_objects > 0)
      && (kslab_cache->nb_free_objects == (kslab_cache->min_free_objects - 1)))
    {
      /* No: allocate a new slab now */
      if (cache_grow(kslab_cache, alloc_flags) != KERNEL_OK)
	{
	  /* Not enough free memory or blocking alloc => undo the
	     allocation */
	  sos_kmem_cache_free(obj_vaddr);
	  ALLOC_RET( (vaddr_t)NULL);
	}
    }

  ALLOC_RET(obj_vaddr);
}


/**
 * Helper function to free the object located at the given address.
 *
 * @param empty_slab is the address of the slab to release, if removing
 * the object causes the slab to become empty.
 */
inline static
ret_t
free_object(vaddr_t vaddr,
	    struct sos_kslab ** empty_slab)
{
  struct sos_kslab_cache *kslab_cache;

  /* Lookup the slab containing the object in the slabs' list */
  struct sos_kslab *slab = sos_kmem_vmm_resolve_slab(vaddr);

  /* By default, consider that the slab will not become empty */
  *empty_slab = NULL;

  /* Did not find the slab */
  if (! slab)
    return -KERNEL_INVALID_VALUE;

  assert(slab->cache);
  kslab_cache = slab->cache;

  /*
   * Check whether the address really could mark the start of an actual
   * allocated object
   */
  /* Address multiple of an object's size ? */
  if (( (vaddr - slab->first_object)
	% kslab_cache->alloc_obj_size) != 0)
    return -KERNEL_INVALID_VALUE;
  /* Address not too large ? */
  if (( (vaddr - slab->first_object)
	/ kslab_cache->alloc_obj_size) >= kslab_cache->nb_objects_per_slab)
    return -KERNEL_INVALID_VALUE;

  /*
   * Ok: we now release the object
   */

  /* Did find a full slab => will not be full any more => move it
     to the head of the slabs' list */
  if (! slab->free)
    {
      list_delete(kslab_cache->slab_list, slab);
      list_add_head(kslab_cache->slab_list, slab);
    }

  /* Release the object */
  list_add_head(slab->free, (struct sos_kslab_free_object*)vaddr);
  slab->nb_free++;
  kslab_cache->nb_free_objects++;
  assert(slab->nb_free <= slab->cache->nb_objects_per_slab);

  /* Cause the slab to be released if it becomes empty, and if we are
     allowed to do it */
  if ((slab->nb_free >= kslab_cache->nb_objects_per_slab)
      && (kslab_cache->nb_free_objects - slab->nb_free
	  >= kslab_cache->min_free_objects))
    {
      *empty_slab = slab;
    }

  return KERNEL_OK;
}


ret_t sos_kmem_cache_free(vaddr_t vaddr)
{
  ret_t retval;
  struct sos_kslab *empty_slab;

  /* Remove the object from the slab */
  retval = free_object(vaddr, & empty_slab);
  if (retval != KERNEL_OK)
    return retval;

  /* Remove the slab and the underlying range if needed */
  if (empty_slab != NULL)
    return cache_release_slab(empty_slab, TRUE);

  return KERNEL_OK;
}


struct sos_kmem_range *
sos_kmem_cache_release_struct_range(struct sos_kmem_range *the_range)
{
  ret_t retval;
  struct sos_kslab *empty_slab;

  /* Remove the object from the slab */
  retval = free_object((vaddr_t)the_range, & empty_slab);
  if (retval != KERNEL_OK)
    return NULL;

  /* Remove the slab BUT NOT the underlying range if needed */
  if (empty_slab != NULL)
    {
      struct sos_kmem_range *empty_range = empty_slab->range;
      assert(cache_release_slab(empty_slab, FALSE) == KERNEL_OK);
      assert(empty_range != NULL);
      return empty_range;
    }

  return NULL;
}

