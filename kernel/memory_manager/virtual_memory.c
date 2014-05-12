#include <arch/x86-all/mmu/paging.h>
#include <arch/all/queue.h>
#include <arch/all/status.h>
#include <arch/all/klibc.h>

#include "physical_memory.h"
#include "virtual_memory.h"

/** The kernel's heap is between these two addresses */
#define KERNEL_VMM_BASE 0x4000 /* 16kB */
#define KERNEL_VMM_TOP  PAGING_MIRROR_VIRTUAL_ADDRESS /* 1GB - 4MB */

TAILQ_HEAD(, memory_range) free_memory_ranges;
TAILQ_HEAD(, memory_range) used_memory_ranges;

struct memory_range
{
	uint32_t base_address;
	uint32_t size;

	TAILQ_ENTRY(memory_range) next;
};


uint32_t heap_start = 0x0;
uint32_t heap_used  = 0x0;
uint32_t heap_limit = 0x3fffff;


void map_pages(uint32_t base_address, 
	       uint32_t top_address,
	       bool_t is_user_page)
{
	uint32_t page_virtual_address;
	uint32_t page_physical_address;
	uint32_t retval;

	for (page_virtual_address = base_address; 
	     page_virtual_address < top_address;
	     page_virtual_address += X86_PAGE_SIZE)
	{
		page_physical_address = physical_memory_page_reference_new();

		assert(page_physical_address != (uint32_t)NULL);

		retval = x86_paging_map(page_physical_address, 
					page_virtual_address,
					is_user_page, 
					VM_FLAG_ATOMIC | VM_FLAG_READ | VM_FLAG_WRITE);

		assert(retval == KERNEL_OK);

		/* This page is already mapped, so it can be unreferenced.
		   It's reference_counter should = 1 at this step 
		   when mapped first time */
		physical_memory_page_unreference(page_physical_address);
	}
}


void unmap_pages(uint32_t base_address, uint32_t top_address)
{
    uint32_t page_virtual_address;

    for (page_virtual_address = base_address;
			page_virtual_address < top_address;
			page_virtual_address += X86_PAGE_SIZE)
    {
		assert(x86_paging_unmap(page_virtual_address) == KERNEL_OK);
    }
}


void *heap_alloc(size_t size)
{
	struct memory_range *mem_range;
	struct memory_range *place;
	uint32_t address;

	if (size <= 0 || TAILQ_EMPTY(&free_memory_ranges))
		return NULL;

	// First round: matching for the exact space
	TAILQ_FOREACH(mem_range, &free_memory_ranges, next)
	{
		if (mem_range->size == size)
		{
			TAILQ_REMOVE(&free_memory_ranges, mem_range, next);
			TAILQ_INSERT_TAIL(&used_memory_ranges, mem_range, next);
			
			return (void *)mem_range->base_address;
		}

	}
	
	// Second round: First-fit method
	/*TAILQ_FOREACH(mem_range, &free_memory_ranges, next)
	{
		if (mem_range->size >= size)
		{
			// Resize the current memory range
			mem_range->size = mem_range->size - size;

			// Allocate a new chunk
			place = (struct memory_range *)heap_start + heap_used;
			heap_used += sizeof(struct memory_range);

			place->base_address = mem_range->base_address + mem_range->size;
			place->size			= size;
			
			// Update the list
			TAILQ_INSERT_TAIL(&used_memory_ranges, place, next);

			return (void *)place->base_address;
		}
	}*/

	// At the end of the used memory
	if ((heap_used + size) < heap_limit)
	{
		place = (struct memory_range *)heap_start + heap_used;
		heap_used += sizeof(struct memory_range);

		address = heap_start + heap_used;
		heap_used += size;

		place->base_address = address;
		place->size = size;

		TAILQ_INSERT_TAIL(&used_memory_ranges, place, next);

		return (void *)place->base_address;
	}

	return NULL;
}


void heap_free(void *address)
{
	struct memory_range *mem_range;

	if (address == NULL)
		return;

	TAILQ_FOREACH(mem_range, &used_memory_ranges, next)
	{
		if (mem_range->base_address == (uint32_t)address)
		{
			TAILQ_REMOVE(&used_memory_ranges, mem_range, next);

			unmap_pages(mem_range->base_address,
				mem_range->base_address + mem_range->size);

			TAILQ_INSERT_TAIL(&free_memory_ranges, mem_range, next);
		}
	}
}


static void heap_setup(uint32_t kernel_top_address)
{
	struct memory_range *free_range;

	heap_start = PAGE_ALIGN_UPPER_ADDRESS(kernel_top_address);

	free_range = (struct memory_range *)heap_start;
	heap_start += sizeof(struct memory_range);

	free_range->base_address = heap_start;
	free_range->size		 = KERNEL_VMM_TOP - heap_start;

	TAILQ_INSERT_TAIL(&free_memory_ranges, free_range, next);

	heap_used = heap_used + sizeof(struct memory_range);
}


void virtual_memory_setup(uint32_t kernel_base,
		uint32_t kernel_top,
		uint32_t ram_size)
{
	TAILQ_INIT(&free_memory_ranges);
	TAILQ_INIT(&used_memory_ranges);

	map_pages(PAGE_ALIGN_UPPER_ADDRESS(kernel_top),
			  PAGE_ALIGN_LOWER_ADDRESS(kernel_top + ram_size * 1024),
			  FALSE);

	heap_setup(kernel_top);
}

