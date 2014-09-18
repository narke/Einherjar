#ifndef _PHYSICAL_MEMORY_H_
#define _PHYSICAL_MEMORY_H_

/**
 * @file physical_memory.h
 * @date 2009, 2013
 * @license MIT License
 *  
 * Memory's pysical pages management
 */


#include <lib/types.h>


/** Each page is 4kb */
#define X86_PAGE_SIZE  (4*1024)

/** A page is 4kb, 4 kB = 2^12 B */
#define X86_PAGE_SHIFT 12

/** Page's mask */
#define X86_PAGE_MASK  ((1<<12) - 1)


/**
 * BIOS memory area reserved for video memory use.
 */
#define BIOS_VIDEO_START 0xa0000
#define BIOS_VIDEO_END   0x100000


/** Align on a boundary (MUST be a power of 2), so that return value <= val */
#define __PAGE_ALIGN_LOWER(value, boundary) \
  (((unsigned)(value)) & (~((boundary)-1)))
 
/** Align on a boundary (MUST be a power of 2), so that return value >= val */
#define __PAGE_ALIGN_UPPER(value, boundary) \
  ({ unsigned int __boundary=(boundary); \
     (((((unsigned)(value))-1) & (~(__boundary - 1))) + __boundary); })

/** Check whether val is aligned on a boundary (MUST be a power of 2) */
#define __IS_ALIGNED(value, boundary) \
  ( 0 == (((unsigned)(value)) & ((boundary)-1)) )

#define PAGE_ALIGN_LOWER_ADDRESS(value) \
  __PAGE_ALIGN_LOWER((value), X86_PAGE_SIZE)
#define PAGE_ALIGN_UPPER_ADDRESS(value) \
  __PAGE_ALIGN_UPPER((value), X86_PAGE_SIZE)
#define IS_PAGE_ALIGNED(value) \
  __IS_ALIGNED((value), X86_PAGE_SIZE)
  
#define BIOS_N_VIDEO_START 0xa0000
#define BIOS_N_VIDEO_END   0x100000

/**
 * Setup the management of physical pages.
 * It preserves BIOS and Video address ranges from memory 
 * allocation requests.
 * We use a "flat memory model" so virtual address == physical address
 * 
 * @param ram_size Amount of the RAM
 *
 * @param kernel_core_base Kernel's lowest address for identity mapping
 *
 * @param kernel_core_top Kernel's top address for identity mapping
 *
 * @param initrd_start Starting address of RAMFS
 *
 * @param initrd_end End address of RAMFS
 * 
 * TODO: 
 * kernel_core_base -> kernel_identity_mapping_bottom
 * kernel_core_top -> kernel_identity_mapping_top
 */
uint16_t physical_memory_setup(uint32_t ram_size,
		/* out */paddr_t *kernel_core_base,
		/* out */paddr_t *kernel_core_top,
		uint32_t initrd_start,
		uint32_t initrd_end);

/**
 * Get a free page.
 * 
 * @return The physical address of the allocated physical page, 
 * or NULL if no one page is available
 * 
 * @note The allocated page's reference count equals to 1.
 */
uint32_t physical_memory_page_reference_new();


/**
 * Increment the reference counter of the physical page
 * located at the specified address.
 *
 * @param page_physical_address address of a physical page
 *
 * @return TRUE = the page was in use
 *         FALSE = the page was free 
 *         <0 = the page address is invalid.
 */
uint32_t physical_memory_page_reference_at(uint32_t page_physical_address);


/**
 * Decrement the reference counter of the physical page. 
 * The page is freed when the reference count reaches 0.
 *
 * @param page_physical_address address of a physical page
 *
 * @return TRUE = the page was freed
 * 		   FALSE = the page is still in use
 * 	 	   <0 = the page address is invalid
 */
uint16_t physical_memory_page_unreference(uint32_t page_physical_address);


#endif // _PHYSICAL_MEMORY_H_

