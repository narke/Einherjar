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

#include <arch/all/types.h>

/** Virtual address access rights */
#define VM_FLAG_READ  (1<<0)
#define VM_FLAG_WRITE (1<<1)
#define VM_FLAG_EXEC  (1<<2) // Not supported on x86

/** Virtual address where mirroring is taking place */
#define PAGING_MIRROR_VIRTUAL_ADDRESS 0x3fc00000 // 1GB - 4MB

/** Space reserved for mirroring in the virtual address space */
#define PAGING_MIRROR_SIZE (1 << 22)	// 1 PD = 1024 Page Tables = 4MB

/** Setting up paging */
uint16_t x86_paging_setup(paddr_t identity_mapping_base,
		  paddr_t identity_mapping_top);

/** Map a virtual address to a physical address */
uint16_t x86_paging_map(paddr_t page_physical_address, 
		    vaddr_t page_virtual_address,
		    bool_t is_user_page,
		    uint32_t flags);

/** Unmap a virtual address */	
uint16_t x86_paging_unmap(vaddr_t virtual_address); 


#endif // _PAGING_H_
