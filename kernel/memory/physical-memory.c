#include <lib/libc.h>
#include <lib/status.h>
#include <lib/queue.h>

#include "physical-memory.h"

struct physical_page_descriptor
{
	/** The physical address of the page */
	uint32_t physical_address;

	/** Number of references to this physical page.
	* > 0: the page is in the used pages list
	* = 0: the page is in the free pages list
	*/
	uint32_t reference_counter;

	TAILQ_ENTRY(physical_page_descriptor) next;
};

TAILQ_HEAD(, physical_page_descriptor) used_pages_head;

TAILQ_HEAD(, physical_page_descriptor) free_pages_head;



/** Kernel beginning marker 
 * @see linker.ld */
extern char __kernel_start;

/** Kernel end marker 
 * @see linker.ld */
extern char __kernel_end;

  
static struct physical_page_descriptor *g_physical_page_descriptor_array;


/** low physical address*/
static uint32_t g_physical_memory_bottom;
/** high physical address*/
static uint32_t g_physical_memory_top;

/** Number of total pages */
static uint32_t g_physical_memory_total_pages;
/** Number of used pages*/
static uint32_t g_physical_memory_used_pages;



ret_t physical_memory_setup(uint32_t ram_size, 
		paddr_t *out_kernel_base, 
		paddr_t *out_kernel_top,
		uint32_t initrd_start,
		uint32_t initrd_end)
{
	uint32_t higher_end_address;
	uint32_t physical_page_address;
	struct physical_page_descriptor *physical_page_descr;

	/** Physical pages descriptors array's address */	
	higher_end_address = (initrd_end > ((uint32_t)&__kernel_end)) ? initrd_end : (uint32_t)&__kernel_end;
	#define PAGE_DESCRIPTORS_ARRAY_ADDRESS PAGE_ALIGN_UPPER_ADDRESS(higher_end_address)

	/* Initialize used pages circular queue. */
	TAILQ_INIT(&used_pages_head);    

	/* Initialize free pages circular queue. */
	TAILQ_INIT(&free_pages_head);  

	/* Ensure that ram size is page aligned.
	   We may lose at most a page.
	 */
	ram_size = PAGE_ALIGN_LOWER_ADDRESS(ram_size);

	/* In the beginning no pages are used */
	g_physical_memory_total_pages = 0;
	g_physical_memory_used_pages = 0;

	/* Update the addresses managed by the physical memory allocator */
	*out_kernel_base = PAGE_ALIGN_LOWER_ADDRESS((uint32_t)(& __kernel_start));
	*out_kernel_top = PAGE_DESCRIPTORS_ARRAY_ADDRESS
			+ PAGE_ALIGN_UPPER_ADDRESS((ram_size >> X86_PAGE_SHIFT)
			* sizeof(struct physical_page_descriptor));

	/* Is there enough memory to fit the kenel? */
	if ( *out_kernel_top > ram_size )
	{
		/* No, oops! */
		return -KERNEL_NO_MEMORY;
	}

	/* The first page is not available in order to signal 
	   that no page is available by returning address 0 */
	g_physical_memory_bottom = X86_PAGE_SIZE;
	g_physical_memory_top  = ram_size;

	/* Setup the page descriptor arrray */
	g_physical_page_descriptor_array
			= (struct physical_page_descriptor *)
			PAGE_DESCRIPTORS_ARRAY_ADDRESS;

	enum { MARK_RESERVED, MARK_FREE, MARK_KERNEL, MARK_HARDWARE } action;


	/* Scan the list of physical pages */
	for (physical_page_address = 0,
		physical_page_descr = g_physical_page_descriptor_array;

		physical_page_address < g_physical_memory_top;

		physical_page_address += X86_PAGE_SIZE, 
		physical_page_descr++)
	{
		memset(physical_page_descr, 0x0, sizeof(struct physical_page_descriptor));

		/* Init the page descriptor for this page */
		physical_page_descr->physical_address = physical_page_address;


		/* Reserved : 0 ... base */
		if (physical_page_address < g_physical_memory_bottom)
		{
			action = MARK_RESERVED;
		}

		/* Free : base ... BIOS */
		else if ((physical_page_address >= g_physical_memory_bottom)
			&& (physical_page_address < BIOS_N_VIDEO_START))
		{	
			action = MARK_FREE;
		}

		/* Used : BIOS */
		else if ((physical_page_address >= BIOS_N_VIDEO_START)
			&& (physical_page_address < BIOS_N_VIDEO_END))
		{
			action = MARK_HARDWARE;
		}

		/* Free : BIOS ... kernel */
		else if ((physical_page_address >= BIOS_N_VIDEO_END)
			&& (physical_page_address < (uint32_t) (& __kernel_start)))
		{
			action = MARK_FREE;
		}

		/* Used : Kernel code/data/bss + physcal page descr array */
		else if ((physical_page_address >= *out_kernel_base)
			&& (physical_page_address < *out_kernel_top))
		{
			// Initrd is also in this range
			action = MARK_KERNEL;
		}

		/* Free : first page of descr ... end of RAM */
		else
		{
			action = MARK_FREE;
		}

		/* Actually does the insertion in the used/free page lists */
		g_physical_memory_total_pages++;


		switch (action)
		{
			case MARK_FREE://printf("Action = %d\n", action);
				physical_page_descr->reference_counter = 0;
				TAILQ_INSERT_TAIL(&free_pages_head, physical_page_descr, next);
				break;

			case MARK_KERNEL:
			case MARK_HARDWARE:
				physical_page_descr->reference_counter = 1;
				TAILQ_INSERT_TAIL(&used_pages_head, physical_page_descr, next);
				g_physical_memory_used_pages++;
				break;

			default:
				/* Reserved page: nop */
				break;
		}
	}

	return KERNEL_OK;
}


uint32_t physical_memory_page_reference_new()
{
	struct physical_page_descriptor *physical_page_descr;
	
	if (TAILQ_EMPTY(&free_pages_head))
		return (uint32_t)NULL;

	/* Get a free page */
	physical_page_descr = TAILQ_FIRST(&free_pages_head);
	TAILQ_REMOVE(&free_pages_head, physical_page_descr, next);

	/* The page should not to be already used */
	assert(physical_page_descr->reference_counter == 0);

	/* Mark the page as used */
	physical_page_descr->reference_counter++;

	/* Put the page in the used pages list */
	TAILQ_INSERT_TAIL(&used_pages_head, physical_page_descr, next);

	g_physical_memory_used_pages++;

	return physical_page_descr->physical_address;
}


/**
 * Get the physical page descriptor for the given
 * physical page address.
 *
 * @return NULL when an invalid address is provided
 */
inline static struct physical_page_descriptor *
physical_memory_get_page_descriptor_at_address(uint32_t page_physical_address)
{
	/* Don't handle non-page-aligned addresses */
	if (page_physical_address & X86_PAGE_MASK)
		return NULL;
  
	/* Check the requested memory address range's validity */
	if ((page_physical_address < g_physical_memory_bottom) 
		|| (page_physical_address >= g_physical_memory_top))
		return NULL;

	return g_physical_page_descriptor_array 
			+ (page_physical_address >> X86_PAGE_SHIFT);
}


ret_t physical_memory_page_reference_at(uint32_t page_physical_address)
{
	struct physical_page_descriptor *physical_page_descr
		= physical_memory_get_page_descriptor_at_address(page_physical_address);

	if (!physical_page_descr)
		return -KERNEL_INVALID_VALUE;

	/* Increment the reference count for the page */
	physical_page_descr->reference_counter++;

    	/* Does this page was already used? */
    	if (physical_page_descr->reference_counter == 1)
    	{
    		/* No, the page is being used only from now so remove it from free pages list */
		TAILQ_REMOVE(&free_pages_head, physical_page_descr, next);

		/* And move it to used pages list */
		TAILQ_INSERT_TAIL(&used_pages_head, physical_page_descr, next);
		g_physical_memory_used_pages++;

		/* The page is newly referenced */
		return FALSE;
	}

	/* The page was already referenced */
	return TRUE;
}


ret_t physical_memory_page_unreference(uint32_t page_physical_address)
{
	/* By default we assume that the page is still used */
	uint32_t retval = FALSE;

	struct physical_page_descriptor *physical_page_descr
		= physical_memory_get_page_descriptor_at_address(page_physical_address);

	if (!physical_page_descr)
		return -KERNEL_INVALID_VALUE;

	/* Ensure that the page is in the used list before doing anything */
	if (physical_page_descr->reference_counter <= 0)
		return -KERNEL_INVALID_VALUE;

	/* Unreference the page by decrementing it's reference counter */
	physical_page_descr->reference_counter--;

	/* Must this page be freed? */
	if (physical_page_descr->reference_counter == 0)
    	{
		/* Yes, remove it from used pages */
		TAILQ_REMOVE(&used_pages_head, physical_page_descr, next);

		g_physical_memory_used_pages--;

		/* And move it to free pages list */
		TAILQ_INSERT_TAIL(&free_pages_head, physical_page_descr, next);

		/* Indicates that the unreferencing operation ended successfully */
		retval = TRUE;
	}

	/* The page was already referenced by someone */
	return retval;
}


