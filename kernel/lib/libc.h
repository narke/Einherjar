#ifndef _LIBC_H_
#define _LIBC_H_

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
#define assert(expression)												\
	({																	\
		int result = (int)(expression);									\
		if (!result)													\
		{																\
			/* Disable interrupts on x86 */								\
			asm("cli\n");												\
			printf("%s@%s:%d Assertion: " # expression " - failed\n",	\
				__PRETTY_FUNCTION__, __FILE__, __LINE__);				\
			/* Infinite loop and x86 processor halting */				\
			while (1) asm("hlt");										\
		}																\
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

/** Copy memory area */
void *memcpy(void *dst, const void *src, register unsigned int size );

/* String copy */
char *strzcpy(register char *dst, register const char *src, register int len);

/**
 * Allocate memory
 * 
 * @param size Number of bytes to allocate
 * @return The address of the allocated memory region
 */
void *malloc(size_t size);

/**
 * Deallocate memory
 *
 * @param ptr Address to release
 */
void free(void *ptr);

void display_fatal_error();

#endif // _LIBC_H_
