#ifndef _SEGMENT_H_
#define _SEGMENT_H_

/**
 * @file segment.h
 * @author Konstantin Tcholokachvili
 * @date 2013
 * @license MIT License
 * 
 * x86 Segment
 */

/** Unused by the processor */
#define NULL_SEGMENT        0
#define KERNEL_CODE_SEGMENT 1
#define KERNEL_DATA_SEGMENT 2

/**
 * Builds a value for a segment register
 */
#define X86_BUILD_SEGMENT_REGISTER_VALUE(segment_index) \
	( (0) /* Descriptor Priviledge Level 0 */           \
	| (0) /* Not in LDT */                              \
	| ((segment_index) << 3) )
	

#endif // _SEGMENT_H_
