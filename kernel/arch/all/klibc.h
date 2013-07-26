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
 * Formatted display of numbers and strings
 * 
 * @format Describes the format: %d,%x or %s
 * @... Variable number of variables ;-)
 */
void printf(const char *format, ...);

/**
 * Assert an expression
 *
 * @param expression The expression to evaluate 
 */
void assert(uint32_t expression);

/**
 * Set the content of a memory zone to a specific value
 *
 * @param dst Memory zone destination
 * @param c Specific value which will written to the memory zone
 * @param length the size of the memory zone
 */
void *memset(void *dst, sint32_t c, uint32_t length);

#endif // _KLIBC_H_
