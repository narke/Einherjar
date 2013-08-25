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
#define NULL_SEGMENT 		0
#define KERNEL_CODE_SEGMENT	1
#define KERNEL_DATA_SEGMENT 	2
#define USER_CODE_SEGMENT 	3
#define USER_DATA_SEGMENT 	4
/** Kernel TSS for CPL3 -> CPL0 privilege change */
#define KERNEL_TSS_SEGMENT	5

#define DPL0 0x00
#define DPL3 0x03

#define RING0 0x00
#define RING3 0x03

/**
 * Builds a value for a segment register
 */
#define X86_BUILD_SEGMENT_REGISTER_VALUE(descriptor_privilege_level, segment_index) \
  ( (((descriptor_privilege_level) & 0x3)	<< 0) \
	| (0) 					      \
	| ((segment_index)               	<< 3) )
	

#endif // _SEGMENT_H_
