#ifndef _PHYSICAL_MEMORY_H_
#define _PHYSICAL_MEMORY_H_

/**
 * @file physical_memory.h
 * @date 2009, 2013, 2017
 * @license MIT License
 *
 * Memory's pysical pages management
 */

#include <lib/types.h>
#include <lib/queue.h>

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

#define PAGE_ALIGN_DOWN(value) \
  __PAGE_ALIGN_LOWER((value), X86_PAGE_SIZE)
#define PAGE_ALIGN_UP(value) \
  __PAGE_ALIGN_UPPER((value), X86_PAGE_SIZE)
#define IS_PAGE_ALIGNED(value) \
  __IS_ALIGNED((value), X86_PAGE_SIZE)


/**
 * Setup the management of physical pages.
 * It preserves BIOS and Video address ranges from memory
 * allocation requests.
 * We use a "flat memory model" so virtual address == physical address
 *
 * @param ram_size Amount of the RAM
 *
 * @param kernel_core_top Kernel's top address for identity mapping
 *
 * @param initrd_start Start address of RAMFS
 *
 * @param initrd_end End address of RAMFS
 *
 */
void physical_memory_setup(uint32_t ram_size,
	uint32_t initrd_start,
	uint32_t initrd_end);

/** Allocate memory on the heap */
void *heap_alloc(size_t size);

/** Free memory by releasing some heap */
void heap_free(void *ptr);

#endif // _PHYSICAL_MEMORY_H_
