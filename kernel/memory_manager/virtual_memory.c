#include <arch/x86-all/mmu/paging.h>
#include <arch/all/queue.h>
#include <arch/all/status.h>
#include <arch/all/klibc.h>

#include "physical_memory.h"
#include "virtual_memory.h"

/** The kernel's heap is between these two addresses */
#define KERNEL_VMM_BASE 0x4000 /* 16kB */
#define KERNEL_VMM_TOP  PAGING_MIRROR_VIRTUAL_ADDRESS /* 1GB - 4MB */

SLIST_HEAD(, memory_range) free_memory_range_head;
SLIST_HEAD(, memory_range) used_memory_range_head;

// Heap's address will start at kernel's top address, see vmm_setup()
uint32_t g_heap = 0;

struct memory_range
{
	uint32_t base_address;
	uint32_t size;

	SLIST_ENTRY(memory_range) next;
};


static void map_pages(uint32_t base_address, 
		      uint32_t top_address)
{
	uint32_t page_virtual_address;
	uint32_t page_physical_address;
	uint32_t retval;

	for (page_virtual_address = base_address; 
	     page_virtual_address < top_address;
	     page_virtual_address = page_virtual_address + X86_PAGE_SIZE)
	{
		page_physical_address = physical_memory_page_reference_new(FALSE);

		assert(page_physical_address != (uint32_t)NULL);

		retval = x86_paging_map(page_physical_address, 
					page_virtual_address,
					FALSE /* Not a user page */, 
					VM_FLAG_ATOMIC
					| VM_FLAG_READ
					| VM_FLAG_WRITE);

		assert(retval == KERNEL_OK);

		/* This page is already mapped, so it can be unreferenced.
		   It's reference_counter should = 1 at this step
		   when mapped first time */
		physical_memory_page_unreference(page_physical_address);
	}
}


static void unmap_pages(uint32_t base_address,
                        uint32_t top_address)
{
        uint32_t page_virtual_address;

        for (page_virtual_address = base_address;
             page_virtual_address < top_address;
             page_virtual_address = page_virtual_address + X86_PAGE_SIZE)
        {
                assert(x86_paging_unmap(page_virtual_address) == KERNEL_OK);
        }
}


static void create_memory_range(bool_t is_free, 
				uint32_t base_virtual_address,
				uint32_t top_virtual_address)
{
	struct memory_range *mem_range;

	mem_range = (struct memory_range *)g_heap;
	
	assert(mem_range != NULL);

	// Update next free address
	g_heap += sizeof(struct memory_range);

	mem_range->base_address = base_virtual_address;
	mem_range->size = (top_virtual_address - base_virtual_address);

	if (is_free)
	{
		SLIST_INSERT_HEAD(&free_memory_range_head, mem_range, next);
	}
	else
	{
		SLIST_INSERT_HEAD(&used_memory_range_head, mem_range, next);
	}	
}


static void* free_memory_range_lookup(uint32_t size)
{
	struct memory_range *mem_range;
	struct memory_range *remainder_mem_range;
	struct memory_range *new_mem_range;

	if (size <= 0)
		return NULL;

	SLIST_FOREACH(mem_range, &free_memory_range_head, next)
	{	
		if (size == mem_range->size)
		{
			SLIST_REMOVE(&free_memory_range_head, mem_range,
                                     memory_range, next);

			SLIST_INSERT_HEAD(&used_memory_range_head, mem_range, next);

			return (void*)mem_range->base_address;
		}
		else if (size < mem_range->size)
		{
			// Detach a new block
			new_mem_range = (struct memory_range *)mem_range;
			new_mem_range->base_address = mem_range->base_address;
			new_mem_range->size = size;

			// Create the remainder block
			remainder_mem_range = (struct memory_range *)
					       mem_range->base_address
					       + size;

			remainder_mem_range->base_address = mem_range->base_address
							    + size;

			remainder_mem_range->size = mem_range->size - size;

			// Remove the original block from free memory list 
			SLIST_REMOVE(&free_memory_range_head,
				     mem_range,
				     memory_range,
				     next);

			// Add the remainder block to the free memory list
                        SLIST_INSERT_HEAD(&free_memory_range_head,
                                          remainder_mem_range,
                                          next);

			// And the new block to used memory list
			SLIST_INSERT_HEAD(&used_memory_range_head,
					  new_mem_range,
					  next);

			return (void*)new_mem_range->base_address;
		}
	}

	/* TODO: if (size > mem_range->size)
		1. coalesce free memory ranges
		2. find a suitable memory region
		3. success: return the pointer
		4. fail: return NULL
        */	

	return NULL;
}


void *heap_alloc(size_t size, uint32_t flags)
{
	struct memory_range *mem_range;	

	if (size <= 0 || SLIST_EMPTY(&free_memory_range_head))
        {
                return NULL;
        }

	// Find a suitable free memory range
        mem_range = free_memory_range_lookup(size);

	if (!mem_range)
		return NULL;

	// Allocate necessary physical pages and map them into virtual memory
	map_pages(mem_range->base_address, mem_range->base_address
					   + mem_range->size);

	return (void *)mem_range->base_address;	
}


void heap_free(void *ptr)
{
	struct memory_range *mem_range;

        if (ptr == NULL)
        {
                return;
        }

        SLIST_FOREACH(mem_range, &used_memory_range_head, next)
        {
                if (mem_range->base_address == (unsigned int)ptr)
                {
                        SLIST_REMOVE(&used_memory_range_head, mem_range, 
                                     memory_range, next);

			unmap_pages(mem_range->base_address, mem_range->base_address
							     + mem_range->size);

                        SLIST_INSERT_HEAD(&free_memory_range_head, 
                                          mem_range, next);
                }
        }
}


void vmm_setup(uint32_t kernel_base, uint32_t kernel_top)
{
	SLIST_INIT(&free_memory_range_head);
	SLIST_INIT(&used_memory_range_head);

	/* The heap starts at free addresses after the end of the kernel
	   and it's datastructures */ 
	g_heap = kernel_top;

	// Map the heap on one page at the beginning
	map_pages(g_heap, g_heap + X86_PAGE_SIZE);

	/* Virtual addresses: 16kB - Video :: FREE */
	create_memory_range(TRUE,
		KERNEL_VMM_BASE,
		PAGE_ALIGN_LOWER_ADDRESS(BIOS_VIDEO_START));

	/* Virtual addresses: Video hardware mapping :: NOT FREE */
	create_memory_range(FALSE,
		PAGE_ALIGN_LOWER_ADDRESS(BIOS_VIDEO_START),
		PAGE_ALIGN_UPPER_ADDRESS(BIOS_VIDEO_END));

	/* Virtual addresses: Video - Kernel :: FREE */
	create_memory_range(TRUE,
		PAGE_ALIGN_UPPER_ADDRESS(BIOS_VIDEO_END),
		PAGE_ALIGN_LOWER_ADDRESS(kernel_base));

	/* Virtual addresses: Kernel code/data :: NOT FREE */
	create_memory_range(FALSE,
	       PAGE_ALIGN_LOWER_ADDRESS(kernel_base),
	       PAGE_ALIGN_UPPER_ADDRESS(kernel_top));

	/* Virtual addresses: kernel_top - heap's first page :: NOT FREE */
	create_memory_range(FALSE,
               PAGE_ALIGN_UPPER_ADDRESS(kernel_top),
               PAGE_ALIGN_LOWER_ADDRESS(kernel_top + X86_PAGE_SIZE));

	/* Virtual addresses: kernel_top - virtual memory top :: FREE */
	create_memory_range(TRUE,
	       PAGE_ALIGN_UPPER_ADDRESS(kernel_top),
	       KERNEL_VMM_TOP);
}

