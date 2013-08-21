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

#include <arch/all/types.h>

/** Setup virtual memory, free and use address ranges */
void vmm_setup(uint32_t kernel_base, uint32_t kernel_top);

/** Allocate memory on the heap */
void *heap_alloc(size_t size, uint32_t flags);

/** Free memory by releasing some heap */
void heap_free(void *ptr);

#endif // _VIRTUAL_MEMORY_H_
