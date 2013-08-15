#ifndef _PAGING_H_
#define _PAGING_H_

/**
 * @file paging.h
 * @author Konstantin Tcholokachvili
 * @date 2013
 *
 * Setting up paging.
 * The MMU maps virtual addresses to physical ones by using a page directory.
 */


/** Setting up paging */
void paging_setup();


#endif // _PAGING_H_
