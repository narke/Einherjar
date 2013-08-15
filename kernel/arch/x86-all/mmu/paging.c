#include <arch/all/types.h>
#include <arch/all/klibc.h>
#include <memory_manager/physical_memory.h>

#include "paging.h"

#define PAGE_TABLE_NR_ENTRIES 	1024
#define PAGE_TABLE_SIZE       	4096	
#define PAGING_FLAG 		0x80000000 // CR0 - bit 31

void paging_setup()
{
	uint32_t *page_directory;
	uint32_t *first_page_table;
	uint32_t page_table_address = 0;
	uint32_t i;
	uint32_t cr0;

	// Setting up the page directory
	page_directory = (uint32_t *)physical_memory_page_reference_new();

	// 0 | 2 means: supervisor level, read/write, not present.
	memset(page_directory, 0 | 2, X86_PAGE_SIZE);

	// Setting up the first page table (offset: 4096 bytes)
	first_page_table = page_directory + PAGE_TABLE_NR_ENTRIES;

	for (i = 0; i < PAGE_TABLE_NR_ENTRIES; i++)
	{
		// Attributes: supervisor level, read/write, present.
		first_page_table[i] = page_table_address | 3;
		// Advance the address to the next page boundary
		page_table_address += PAGE_TABLE_SIZE;
	}
	
	// Filling the page directory
	page_directory[0] = (uint32_t)first_page_table;
	// Attributes: supervisor level, read/write, present
	page_directory[0] |= 3;

	// Enable paging
	// The CR3 register must point to the page directory
	asm volatile("mov %0, %%cr3":: "b"(page_directory));
	// A bit must be "swithed on" in the register cr0 to enable paging
	asm volatile("mov %%cr0, %0": "=b"(cr0)); // read cr0
	cr0 |= PAGING_FLAG;  			  // switching paging bit on
	asm volatile("mov %0, %%cr0":: "b"(cr0)); // write back	
}
