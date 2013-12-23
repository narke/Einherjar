#include <arch/x86-all/mmu/paging.h>
#include <arch/all/queue.h>
#include <arch/all/status.h>
#include <arch/all/klibc.h>

#include "physical_memory.h"
#include "virtual_memory.h"

/** The kernel's heap is between these two addresses */
#define KERNEL_VMM_BASE 0x4000 /* 16kB */
#define KERNEL_VMM_TOP  PAGING_MIRROR_VIRTUAL_ADDRESS /* 1GB - 4MB */

SLIST_HEAD(, memory_range) free_memory_ranges;
SLIST_HEAD(, memory_range) used_memory_ranges;

// Heap's address will start at kernel's top address, see vmm_setup()
uint32_t g_heap = 0;

struct memory_range
{
	uint32_t base_address;
	uint32_t size;

	SLIST_ENTRY(memory_range) next;
};


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
		SLIST_INSERT_HEAD(&free_memory_ranges, mem_range, next);
	}
	else
	{
		SLIST_INSERT_HEAD(&used_memory_ranges, mem_range, next);
	}	
}

static void *memory_block_management(struct memory_range *free_range,
				     uint32_t size,
				     bool_t resize)
{
        struct memory_range *new;
        struct memory_range *remainder;

printf("requested size = %d | size = %d\n", size, free_range->size);
	if (resize == FALSE)
	{
		/* The free memory block meets the exact disired memory
		   allocation size */
		SLIST_REMOVE(&free_memory_ranges, free_range, 
			     memory_range, next);

		SLIST_INSERT_HEAD(&used_memory_ranges, free_range, next);

		return (void*)free_range->base_address;
	}
	else
	{
		/* The memory block must be resized */
		                       
		// Detach a new block
		new = (struct memory_range *)free_range;
		new->base_address = free_range->base_address;
		new->size = size;

		// Create the remainder block
		remainder = (struct memory_range *)free_range->base_address + size;
printf("addr = %x\n", free_range->base_address);
printf("AAA\n");
		remainder->base_address = free_range->base_address + size;
printf("XXX\n");
		remainder->size = free_range->size - size;
printf("YYY\n");
		// Remove the original block from free memory list 
		SLIST_REMOVE(&free_memory_ranges,
			     free_range,
			     memory_range,
			     next);
printf("BBB\n");
		// Add the remainder block to the free memory list
		SLIST_INSERT_HEAD(&free_memory_ranges, remainder, next);

		// And the new block to the used memory list
		SLIST_INSERT_HEAD(&used_memory_ranges, new, next);

		return (void*)new->base_address;
	}
}

static void *free_memory_range_lookup(uint32_t size)
{
	struct memory_range *mem_range;

	if (size <= 0)
		return NULL;

	// Take into account the size of memory_range structure
	size += sizeof(struct memory_range);

	SLIST_FOREACH(mem_range, &free_memory_ranges, next)
	{
		if (size == mem_range->size)
		{
			return memory_block_management(mem_range, size, FALSE);
		}
		else if (size < mem_range->size)
		{
			return memory_block_management(mem_range, size, TRUE);
		}
	}

	return NULL;
}


void *heap_alloc(size_t size, uint32_t flags)
{
	struct memory_range *mem_range;	

	if (size <= 0 || SLIST_EMPTY(&free_memory_ranges))
        {
                return NULL;
        }

	// Find a suitable free memory range
        mem_range = free_memory_range_lookup(size);

	if (!mem_range)
		return NULL;

	// Allocate necessary physical pages and map them into virtual memory
	map_pages(mem_range->base_address, mem_range->base_address
					   + mem_range->size,
					   FALSE);

	return (void *)mem_range->base_address;	
}


void heap_free(void *ptr)
{
	struct memory_range *mem_range;

        if (ptr == NULL)
        {
                return;
        }

        SLIST_FOREACH(mem_range, &used_memory_ranges, next)
        {
                if (mem_range->base_address == (unsigned int)ptr)
                {
                        SLIST_REMOVE(&used_memory_ranges, mem_range, 
                                     memory_range, next);

			unmap_pages(mem_range->base_address, mem_range->base_address
							     + mem_range->size);

                        SLIST_INSERT_HEAD(&free_memory_ranges, 
                                          mem_range, next);
                }
        }
}


static heap_setup(uint32_t heap_start_address)
{
	/* The heap starts at free addresses after the end of the kernel
           and it's datastructures */
	g_heap = PAGE_ALIGN_UPPER_ADDRESS(heap_start_address);

    // Map the heap on one page at the beginning
	map_pages(g_heap, g_heap + X86_PAGE_SIZE, FALSE);
}

void vmm_setup(uint32_t kernel_base, uint32_t kernel_top)
{
	SLIST_INIT(&free_memory_ranges);
	SLIST_INIT(&used_memory_ranges);

	heap_setup(kernel_top);

	/* Virtual addresses: 16kB - Video :: FREE */
	create_memory_range(TRUE,
		KERNEL_VMM_BASE,
		PAGE_ALIGN_LOWER_ADDRESS(BIOS_VIDEO_START));

	/* Virtual addresses: Video hardware mapping :: RESERVED */
	create_memory_range(FALSE,
		PAGE_ALIGN_LOWER_ADDRESS(BIOS_VIDEO_START),
		PAGE_ALIGN_UPPER_ADDRESS(BIOS_VIDEO_END));

	/* Virtual addresses: Video - Kernel :: FREE */
	create_memory_range(TRUE,
		PAGE_ALIGN_UPPER_ADDRESS(BIOS_VIDEO_END),
		PAGE_ALIGN_LOWER_ADDRESS(kernel_base));

	/* Virtual addresses: Kernel code/data :: RESERVED */
	create_memory_range(FALSE,
	       PAGE_ALIGN_LOWER_ADDRESS(kernel_base),
	       PAGE_ALIGN_UPPER_ADDRESS(kernel_top));

	/* Virtual addresses: heap's first page - virtual memory top :: FREE */
	create_memory_range(TRUE,
	       PAGE_ALIGN_UPPER_ADDRESS(kernel_top),
	       KERNEL_VMM_TOP);
}

