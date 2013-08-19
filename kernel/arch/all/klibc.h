#ifndef _KLIBC_H_
#define _KLIBC_H_

/**
 * @file klibc.h
 * @author Konstantin Tcholokachvili
 * @date 2013
 * @license MIT License
 * 
 * Tiny C library for the kernel land 
 */

#include "types.h"

/**
 * Assert an expression
 *
 * @param expression The expression to evaluate 
 */
#define assert(expression) 				\
	({ 						\
		int result = (int)(expression);		\
		if (!result)				\
		{					\
			/* Disable interrupts on x86 */	\
			asm("cli\n");			\
			printf("%s@%s:%d Assertion: " # expression " - failed\n",	\
				 __PRETTY_FUNCTION__, __FILE__, __LINE__);		\
			/* Infinite loop and x86 processor halting */	\
			while (1) asm("hlt");		\
		}					\
	 })
	
 

/**
 * Formatted display of numbers and strings
 * 
 * @format Describes the format: %d,%x or %s
 * @... Variable number of variables ;-)
 */
void printf(const char *format, ...);

/**
 * Set the content of a memory zone to a specific value
 *
 * @param dst Memory destination zone
 * @param c Specific value which will written to the memory zone
 * @param length the size of the memory zone
 */
void *memset(void *dst, sint32_t c, uint32_t length);

/**
 * Copy memory area
 *
 * @param dst Memory destination zone
 * @param src Memory source
 * @param n Number of bytes to copy
 */
void *memcpy(void *dst, const void *src, size_t n);

#endif // _KLIBC_H_
