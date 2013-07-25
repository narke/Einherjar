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

#endif // _KLIBC_H_
