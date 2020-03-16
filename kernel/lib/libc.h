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
#define assert(expression)								\
	({										\
		int result = (int)(expression);						\
		if (!result)								\
		{									\
			/* Disable interrupts on x86 */					\
			asm("cli\n");							\
			printf("%s@%s:%d Assertion: " # expression " - failed\n",	\
				__PRETTY_FUNCTION__, __FILE__, __LINE__);		\
			/* Infinite loop and x86 processor halting */			\
			while (1) asm("hlt");						\
		}																\
	})

/**
 * Panic
 *
 * @param msg Display a message
 */
#define panic(msg)								\
	({									\
		/* Disable interrupts on x86 */					\
		asm("cli\n");							\
		printf("%s@%s:%d " # msg " \n",					\
			__PRETTY_FUNCTION__, __FILE__, __LINE__);		\
		/* Infinite loop and x86 processor halting */			\
		while (1) asm("hlt");						\
	})

/**
 * Determines if a string can be converted to a number
 *
 * @param ptr String representing a number
 * @return TRUE or FALSE
 */
bool_t is_number(char *ptr);

/**
 * Converts a string to an integer
 *
 * @param ptr String representing a number
 * @return Converted string
 */
int atoi(const char *ptr);

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
void *memset(void *dst, int c, size_t length);

/** Copy memory area */
void *memcpy(void *dst, const void *src, register size_t size);

/** String copy */
char *strzcpy(register char *dst, register const char *src, register size_t len);

/** Find the first occurence of a character */
char *strchr(const char *s, int c);

/** Calculate the length of a string */
size_t strlen(const char *s);

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

#endif // _LIBC_H_
