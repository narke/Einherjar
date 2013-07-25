#ifndef _RETURN_VALUES_H_
#define _RETURN_VALUES_H_


/**
 * @file return_values.h
 * @author Konstantin Tcholokachvili
 * @date 2007, 2011, 2013
 */
 
 
#define KERNEL_OK                           0   
#define KERNEL_INVALID_VALUE                1   
#define KERNEL_OPERATION_NOT_SUPPORTED      2   
#define KERNEL_NO_MEMORY                    3
#define KERNEL_BUSY                         4
#define KERNEL_INTERRUPTED                  5	
#define KERNEL_PERMISSION_ERROR             6
#define KERNEL_UNRESOLVED_VIRTUAL_ADDRESS   7
#define KERNEL_INTERNAL_FATAL_ERROR         255 

#endif // _RETURN_VALUES_H_
