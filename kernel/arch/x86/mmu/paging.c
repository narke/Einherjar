#include <lib/libc.h>
#include <lib/status.h>
#include <memory/physical-memory.h>

#include "paging.h"

#define PAGE_TABLE_NR_ENTRIES 	1024
#define PAGE_TABLE_SIZE       	4096	
#define PAGING_FLAG 		0x80000000 // CR0 - bit 31

#define VIRTUAL_ADDRESS_TO_PAGE_DIRECTORY_INDEX(virtual_address) \
  (((unsigned)(virtual_address)) >> 22)

#define VIRTUAL_ADDRESS_TO_PAGE_TABLE_INDEX(virtual_address) \
  ( (((unsigned)(virtual_address)) >> 12) & 0x3ff )

/** Page directory */
struct x86_page_directory
{
	uint32_t present:1;			/**< 1=present in RAM, 0=absent */
	uint32_t rw:1;				/**< 1=read and write, 0=read-only */
	uint32_t mode:1;			/**< 1=user mode page, 0=kernel mode page */
	uint32_t write_through:1;	/**< 1=write-through,  0=write-back */
	uint32_t cache_disable:1; 	/**< 1=caching disabled, 0=caching enabled */
	uint32_t accessed:1;		/**< 1=read from or written to */
	uint32_t reserved:1;		/**< reserved by Intel, must be 0 */
	uint32_t page_size:1;		/**< 1=4kb 0=4MB (2MB if PAE is set) */
	uint32_t global_page:1;		/**< Ignored */
	uint32_t available:3;		/**< freely available for any use */
	uint32_t page_table_base_address:20; /**< physical address of a page directory */
}__attribute__((packed));


/** Page table */
struct x86_page_table
{
	uint32_t present:1;			/**< 1=present in RAM, 0=absent */
	uint32_t rw:1;				/**< 1=read and write, 0=read-only */
	uint32_t mode:1;			/**< 1=user mode page, 0=kernel mode page */
	uint32_t write_through:1;	/**< 1=write-through,  0=write-back */
	uint32_t cache_disable:1;	/**< 1=caching disabled, 0=caching enabled */
	uint32_t accessed:1;		/**< 1=read from or written to */
	uint32_t dirty:1;			/**< 1=has been written */
	uint32_t reserved:1; 		/**< reserved by Intel, must be 0 */
	uint32_t global_page:1;		/**< page table not invalidated in TLB 
								(if PGE is set in CR4) when CR3 is 
								loaded or task switch is made*/
	uint32_t available:3;		/**< freely available for any use */	
	uint32_t page_base_address:20; 	/**< physical address of a page table */
}__attribute__((packed));


/** Flush the TLB */
#define invlpg(virtual_address)											\
  do {																	\
      asm volatile("invlpg %0"::"m"(*((unsigned *)(virtual_address)))); \
  } while(0)



static ret_t identity_mapping(struct x86_page_directory *page_directory,
				 paddr_t physical_address,
				 vaddr_t virtual_address)
{
	uint32_t index_in_pd = VIRTUAL_ADDRESS_TO_PAGE_DIRECTORY_INDEX(virtual_address);
	uint32_t index_in_pt = VIRTUAL_ADDRESS_TO_PAGE_TABLE_INDEX(virtual_address);

	/* Make sure the page table was mapped */
	struct x86_page_table *page_table;

	/* Is the page directory's enytry mapped? */
	if (page_directory[index_in_pd].present)
	{
        /* Yes */
        page_table = (struct x86_page_table*)
			(page_directory[index_in_pd].page_table_base_address << 12);

        /* Is a corresponding entry present in the pages table? */
        if (!page_table[index_in_pt].present)
        {
            /* No, allocate it */
            physical_memory_page_reference_at((uint32_t)page_table);
        }
        else
        {
            /* The previous test must be always true because 
             * the setup function scans pages in increasing order */
            assert(FALSE);
        }
    }
    else
    {
        /* No, allocate a new one */
        page_table = (struct x86_page_table*)physical_memory_page_reference_new();

        if (!page_table)
            return -KERNEL_NO_MEMORY;

        memset((void*)page_table, 0x0, X86_PAGE_SIZE);

        page_directory[index_in_pd].present	= TRUE;
        page_directory[index_in_pd].rw    	= 1;
        page_directory[index_in_pd].page_table_base_address = ((uint32_t)page_table) >> 12;
    }

    /* Map the page in the page table */
    page_table[index_in_pt].present	= 1;
    page_table[index_in_pt].rw   	= 1;  
    page_table[index_in_pt].mode    = 0;
    page_table[index_in_pt].page_base_address   = physical_address >> 12;

    return KERNEL_OK;	
}

ret_t x86_paging_setup(paddr_t identity_mapping_base, paddr_t identity_mapping_top)
{
	struct x86_page_directory *page_directory;
	uint32_t cr0;

	page_directory = (struct x86_page_directory*)
			 physical_memory_page_reference_new();

	paddr_t physical_address;

	memset((void*)page_directory, 0, X86_PAGE_SIZE);

	/* Identity map from identity_mapping_base to identity_mapping_top */
	for (physical_address = identity_mapping_base;
			physical_address < identity_mapping_top;
			physical_address = physical_address + X86_PAGE_SIZE)
    {
        if (identity_mapping(page_directory,
					physical_address,
					physical_address))
        {
            return -KERNEL_NO_MEMORY;
        }
    }

    /* Identity map the video area */
    for (physical_address = BIOS_VIDEO_START;
			physical_address < BIOS_VIDEO_END;
			physical_address = physical_address + X86_PAGE_SIZE)
    {
        if (identity_mapping(page_directory,
					physical_address,
					physical_address))
        {
            return -KERNEL_NO_MEMORY;
        }
    }

    /* Setup the mirroring */
    page_directory[VIRTUAL_ADDRESS_TO_PAGE_DIRECTORY_INDEX(PAGING_MIRROR_VIRTUAL_ADDRESS)].present = TRUE;
    page_directory[VIRTUAL_ADDRESS_TO_PAGE_DIRECTORY_INDEX(PAGING_MIRROR_VIRTUAL_ADDRESS)].rw = 1;
    page_directory[VIRTUAL_ADDRESS_TO_PAGE_DIRECTORY_INDEX(PAGING_MIRROR_VIRTUAL_ADDRESS)].mode  = 0;
    page_directory[VIRTUAL_ADDRESS_TO_PAGE_DIRECTORY_INDEX(PAGING_MIRROR_VIRTUAL_ADDRESS)].page_table_base_address = ((uint32_t)page_directory)>>12;

    /* Enable paging, the CR3 register must point to the page directory */
    asm volatile("mov %0, %%cr3":: "b"(page_directory));
    /* A bit must be "swithed on" in the register cr0 to enable paging */
    asm volatile("mov %%cr0, %0": "=b"(cr0));	// read cr0
    cr0 |= PAGING_FLAG;							// switching paging bit on
    asm volatile("mov %0, %%cr0":: "b"(cr0));	// write back	

    return KERNEL_OK;
}


ret_t x86_paging_map(paddr_t page_physical_address,
			vaddr_t page_virtual_address,
			bool_t is_user_page,
			uint32_t flags)
{
	uint32_t index_in_pd = VIRTUAL_ADDRESS_TO_PAGE_DIRECTORY_INDEX(page_virtual_address);
	uint32_t index_in_pt = VIRTUAL_ADDRESS_TO_PAGE_TABLE_INDEX(page_virtual_address);
  
	/* Get the page directory of the current context */
	struct x86_page_directory *pd = (struct x86_page_directory*)
		(PAGING_MIRROR_VIRTUAL_ADDRESS
		 + X86_PAGE_SIZE * 
		 VIRTUAL_ADDRESS_TO_PAGE_DIRECTORY_INDEX(PAGING_MIRROR_VIRTUAL_ADDRESS));

	/* Address of the page table in the mirroring */
	struct x86_page_table *pt = (struct x86_page_table*) 
		(PAGING_MIRROR_VIRTUAL_ADDRESS + X86_PAGE_SIZE*index_in_pd);

	/* Check mirroring boundaries */
	if ((page_virtual_address >= PAGING_MIRROR_VIRTUAL_ADDRESS)
			&& (page_virtual_address < PAGING_MIRROR_VIRTUAL_ADDRESS + PAGING_MIRROR_SIZE))
		return -KERNEL_INVALID_VALUE;

	/* Is a page present at this address? */
	if (!pd[index_in_pd].present)
	{
		/* No, allocate a new one */
		uint32_t pt_ppage = physical_memory_page_reference_new();
      
		if (!pt_ppage)
			return -KERNEL_NO_MEMORY;

		/* Map the page table in the mirrored page directory */
		pd[index_in_pd].present  = TRUE;
		pd[index_in_pd].rw    = 1; /* Ignored in supervisor mode, see IA32 Manual, Volume 3, section 4.12 */
		pd[index_in_pd].mode  = (is_user_page)?1:0;
		pd[index_in_pd].page_table_base_address = ((uint32_t)pt_ppage) >> 12;
      
		/* Invalidate TLB for the page we just added */
		invlpg(pt);
     
		/* Reset this new page table */
		memset((void*)pt, 0x0, X86_PAGE_SIZE);
	}
	/* Is a page present at this address? */ 
	else if (!pt[index_in_pt].present)
	{
		/* No, allocate a new entry in the page table 
		   and increase its reference count */
		physical_memory_page_reference_at(pd[index_in_pd].page_table_base_address << 12);
	}
    else
    {
        /* Yes, unmap it */
        physical_memory_page_unreference(pt[index_in_pt].page_base_address << 12);
    }
    
	/* Map the page in the page table */
	pt[index_in_pt].present = TRUE;
	pt[index_in_pt].rw		= (flags & VM_FLAG_WRITE)?1:0;	
	pt[index_in_pt].mode    = 0;
	pt[index_in_pt].page_base_address	= page_physical_address >> 12;

	/* It will be mapped to the current address space */
	physical_memory_page_reference_at(page_physical_address);

	/* Invalidate TLB for the page we just added */
	invlpg(page_virtual_address);

	return KERNEL_OK;
}


ret_t x86_paging_unmap(vaddr_t page_virtual_address)
{
	uint32_t pt_unref_retval;

	uint32_t index_in_pd = VIRTUAL_ADDRESS_TO_PAGE_DIRECTORY_INDEX(page_virtual_address);
	uint32_t index_in_pt = VIRTUAL_ADDRESS_TO_PAGE_TABLE_INDEX(page_virtual_address);
  
	/* Get the PD of the current context */
	struct x86_page_directory *pd = (struct x86_page_directory*)
		(PAGING_MIRROR_VIRTUAL_ADDRESS
		 + X86_PAGE_SIZE
		 * VIRTUAL_ADDRESS_TO_PAGE_DIRECTORY_INDEX(PAGING_MIRROR_VIRTUAL_ADDRESS));

	/* Get the address of the page table in the mirroring */
	struct x86_page_table *pt = (struct x86_page_table*)
		(PAGING_MIRROR_VIRTUAL_ADDRESS + X86_PAGE_SIZE*index_in_pd);

	/* Is a page mapped at this address ? */
	if (!pd[index_in_pd].present)
	{
		/* No */
		return -KERNEL_INVALID_VALUE;
	}

	/* Is a page mapped at this address ? */
	if (! pt[index_in_pt].present)
	{
		/* No */
		return -KERNEL_INVALID_VALUE;
	}

	/* Check the mirroring boundaries */
	if ((page_virtual_address >= PAGING_MIRROR_VIRTUAL_ADDRESS)
		&& (page_virtual_address < PAGING_MIRROR_VIRTUAL_ADDRESS + PAGING_MIRROR_SIZE))
		return -KERNEL_INVALID_VALUE;

	/* Free the physical page */
	physical_memory_page_unreference(pt[index_in_pt].page_base_address << 12);

	/* Unmap the page in the page table */
	memset(pt + index_in_pt, 0x0, sizeof(struct x86_page_table));

	/* Invalidate the TLB for the page we just unmapped */
	invlpg(page_virtual_address);

	/* Free this page table's entry, it may free the page table */
	pt_unref_retval = physical_memory_page_unreference(pd[index_in_pd].page_table_base_address << 12);
  
	assert(pt_unref_retval >= 0);

	/* Is the page table completely unused? */
	if (pt_unref_retval > 0)
	{
		/* Yes, release the PDE */
		memset(pd + index_in_pd, 0x0, sizeof(struct x86_page_directory));
      
		/* Update the TLB */
		invlpg(pt);
	}
	
	return KERNEL_OK;  
}
