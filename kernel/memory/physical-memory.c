#include <lib/libc.h>
#include <lib/status.h>
#include <lib/queue.h>

#include "physical-memory.h"

TAILQ_HEAD(, memory_range) free_memory_ranges;
TAILQ_HEAD(, memory_range) used_memory_ranges;

struct memory_range
{
	uint32_t base_address;
	uint32_t size;

	TAILQ_ENTRY(memory_range) next;
};

uint32_t heap = 0x0;

// Kernel beginning marker  @see linker.ld
extern char __kernel_start;

// Kernel end marker @see linker.ld
extern char __kernel_end;


void physical_memory_setup(uint32_t ram_size,
	uint32_t initrd_start,
	uint32_t initrd_end)
{
	struct memory_range *r1, *r2, *r3, *r4, *r5, *r6, *r7, *r8;
	paddr_t *kernel_top_address = (paddr_t *)&initrd_end;

	TAILQ_INIT(&free_memory_ranges);
	TAILQ_INIT(&used_memory_ranges);

	heap = PAGE_ALIGN_UP(*kernel_top_address);

	// Reserved : 0 ... base
	r1 = (struct memory_range *)heap;
	heap += sizeof(struct memory_range);

	r1->base_address = X86_PAGE_SIZE;
	r1->size = X86_PAGE_SIZE;

	// Free : base ... BIOS
	r2 = (struct memory_range *)heap;
	heap += sizeof(struct memory_range);

	r2->base_address = r2->base_address;
	r2->size = BIOS_VIDEO_START - r2->base_address;

	TAILQ_INSERT_TAIL(&free_memory_ranges, r2, next);

	// Used : BIOS
	r3 = (struct memory_range *)heap;
	heap += sizeof(struct memory_range);

	r3->base_address = BIOS_VIDEO_START;
	r3->size = BIOS_VIDEO_END - BIOS_VIDEO_START;

	TAILQ_INSERT_TAIL(&free_memory_ranges, r3, next);

	// Free : BIOS ... kernel
	r4 = (struct memory_range *)heap;
	heap += sizeof(struct memory_range);

	r4->base_address = BIOS_VIDEO_END;
	r4->size = (uint32_t)(&__kernel_start) - BIOS_VIDEO_END;

	TAILQ_INSERT_TAIL(&free_memory_ranges, r4, next);

	// Used: Initrd
	r5 = (struct memory_range *)heap;
	heap += sizeof(struct memory_range);

	r5->base_address = initrd_start;
	r5->size = initrd_end - initrd_start;

	TAILQ_INSERT_TAIL(&free_memory_ranges, r5, next);

	// Used : Kernel code/data/bss + physcal page descr array
	r6 = (struct memory_range *)heap;
	heap += sizeof(struct memory_range);

	r6->base_address = (paddr_t)(& __kernel_start);
	r6->size = (paddr_t)(& __kernel_end);

	TAILQ_INSERT_TAIL(&free_memory_ranges, r6, next);

	// Free : first page of descr ... end of RAM
	r7 = (struct memory_range *)heap;
	heap += sizeof(struct memory_range);

	r7->base_address = (paddr_t)(&__kernel_end);
	r7->size = initrd_start - (paddr_t)(&__kernel_end);

	r8 = (struct memory_range *)heap;
	heap += sizeof(struct memory_range);

	// Ensure that the RAM size is page aligned
	ram_size = PAGE_ALIGN_DOWN(ram_size);

	r8->base_address = initrd_end;
	r8->size = ram_size - initrd_end;

	TAILQ_INSERT_TAIL(&free_memory_ranges, r7, next);
	TAILQ_INSERT_TAIL(&free_memory_ranges, r8, next);
}


void *heap_alloc(size_t size)
{
	struct memory_range *mem_range, *place;

	if (size == 0 || TAILQ_EMPTY(&free_memory_ranges))
		return NULL;

	// First round: matching for the exact space
	TAILQ_FOREACH(mem_range, &free_memory_ranges, next)
	{
		if (mem_range->size == size)
		{
			TAILQ_REMOVE(&free_memory_ranges, mem_range, next);
			TAILQ_INSERT_TAIL(&used_memory_ranges, mem_range, next);

			return (void *)mem_range->base_address;
		}
	}

	// Second round: First-fit method
	TAILQ_FOREACH(mem_range, &free_memory_ranges, next)
	{
		if (mem_range->size >= size)
		{
			// Resize the current memory range
			mem_range->size = mem_range->size - size;

			// Allocate a new chunk
			place = (struct memory_range *)heap;
			heap += sizeof(struct memory_range);

			place->base_address = mem_range->base_address + mem_range->size;
			place->size = size;

			// Update the list
			TAILQ_INSERT_TAIL(&used_memory_ranges, place, next);

			return (void *)place->base_address;
		}
	}

	return NULL;
}


void heap_free(void *address)
{
	struct memory_range *mem_range;

	if (address == NULL)
		return;

	TAILQ_FOREACH(mem_range, &used_memory_ranges, next)
	{
		if (mem_range->base_address == (uint32_t)address)
		{
			TAILQ_REMOVE(&used_memory_ranges, mem_range, next);
			TAILQ_INSERT_TAIL(&free_memory_ranges, mem_range, next);
		}
	}
}
