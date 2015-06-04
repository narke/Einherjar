#ifndef _VIRTUAL_MEMORY_H_
#define _VIRTUAL_MEMORY_H_

/**
 * @file virtual_memory.h
 * @author Konstantin Tcholokachvili
 * @date 2013
 * @license MIT License
 *  
 * Virtual memory and heap allocation.
 */

#include <lib/types.h>

/** Setup virtual memory, free and use address ranges */
void virtual_memory_setup(uint32_t kernel_top, uint32_t ram_size);

/** Allocate memory on the heap */
void *heap_alloc(size_t size);

/** Free memory by releasing some heap */
void heap_free(void *ptr);

void map_pages(uint32_t base_address,
			   uint32_t top_address,
			   bool_t is_user_page);

void unmap_pages(uint32_t base_address,
				 uint32_t top_address);

#endif // _VIRTUAL_MEMORY_H_

