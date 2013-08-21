#ifndef _MEMORY_MAPPING_H_
#define _MEMORY_MAPPING_H_

/**
 * @file memory_mapping.h
 * @author Konstantin Tcholokachvili
 * @date 2013
 * @license MIT License
 *  
 * Implementation of mmap() and munmap().
 * Credits: https://github.com/grahamedgecombe/arc for inspiration.
 */

#include <arch/all/types.h>

/* make it so when dlmalloc aborts a kernel panic is triggered */
#define ABORT printf("dlmalloc abort()")

/* use assert() assert.h for assertions */
#define ABORT_ON_ASSERT_FAILURE 0

/* Due to lack of beer these header files weren't implemented */
#define LACKS_ERRNO_H
#define LACKS_STDLIB_H
#define LACKS_TIME_H
#define LACKS_STRING_H
#define LACKS_STRINGS_H
#define LACKS_SYS_TYPES_H
#define LACKS_SYS_MMAN_H
#define LACKS_SYS_PARAM_H
#define LACKS_FCNTL_H
#define LACKS_UNISTD_H
#define LACKS_SCHED_H

/* Stats require stdio.h which we don't support */
#define NO_MALLOC_STATS 1

/* Don't use built-in spinlocks */
#define USE_LOCKS           0
#define USE_SPIN_LOCKS      0

/* Prefix function names with dl */
#define USE_DL_PREFIX 1

/* We don't support errno so just nop instead */
#define MALLOC_FAILURE_ACTION

/* We only support mmap-like calls */
#define HAVE_MORECORE 0
#define HAVE_MMAP     1
#define HAVE_MREMAP   0
#define MMAP_CLEARS   0

/* Hard-coded page size of 4k */
#define FRAME_SIZE 4096
#define malloc_getpagesize FRAME_SIZE

/* Some error codes */
#define ENOMEM 1
#define EINVAL 2

/* Minimal definitions for translating mmap() to heap_*() */
#define MAP_ANONYMOUS 0x1
#define MAP_PRIVATE   0x2

#define MAP_FAILED ((void *) -1)

#define PROT_NONE  0x0
#define PROT_READ  0x1
#define PROT_WRITE 0x2
#define PROT_EXEC  0x4

/** Memory access flags */
typedef enum
{
	VM_R = 0x1, /**< Readable (always the case on x86) */
	VM_W = 0x2, /**< Writable */
	VM_X = 0x4  /**< Executable (on x86 lack of this flag sets the NX bit)*/
} vm_access_t;


/** Map file or device into memory region */
void *mmap(void *address, size_t length, int protection, int flags,
	   int fd, off_t offset);

/** Unmap file or device from a memory region */
int munmap(void *address, size_t length);

// Include dlmalloc.h here to get rid of constants redefinitions
#include "dlmalloc.h"

#endif // _MEMORY_MAPPING_H_
