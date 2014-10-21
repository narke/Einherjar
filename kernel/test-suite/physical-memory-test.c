#include <lib/types.h>
#include <lib/libc.h>
#include <lib/queue.h>
#include <arch/x86-pc/io/vga.h>
#include <memory/physical-memory.h>

#include "physical-memory-test.h"

#define MY_PPAGE_NUM_INT 511

struct phys_page
{
        uint32_t before[MY_PPAGE_NUM_INT];
        TAILQ_ENTRY(phys_page) next;
        uint32_t after[MY_PPAGE_NUM_INT];
};


void test_physical_memory(void)
{
        TAILQ_HEAD(, phys_page) phys_pages_head;

        struct phys_page *phys_page;

        uint32_t nb_allocated_physical_pages = 0;
	uint32_t nb_free_physical_pages = 0;

        TAILQ_INIT(&phys_pages_head);

        printf("\n\n++ Physical memory allocaion/deallocation test! ++\n");

	// Test the allocation
        while ((phys_page = (struct phys_page*)physical_memory_page_reference_new()) != NULL)
        {
                int i;
                nb_allocated_physical_pages++;

                vga_set_position(0, 7);
                printf("Can allocate %d pages\n", nb_allocated_physical_pages);

                for (i = 0 ; i < MY_PPAGE_NUM_INT ; i++)
		{
                        phys_page->before[i] = (uint32_t)phys_page;
			phys_page->after[i]  = (uint32_t)phys_page;
		}

                TAILQ_INSERT_TAIL(&phys_pages_head, phys_page, next);
        }

	// Test the deallocation
	TAILQ_FOREACH(phys_page, &phys_pages_head, next)
        {
                int i;

                for (i = 0 ; i < MY_PPAGE_NUM_INT ; i++)
                {
                        if ((phys_page->before[i] !=  (uint32_t)phys_page)
                            || (phys_page->after[i] !=  (uint32_t)phys_page))
                        {
                                printf("Page overwritten\n");
                                return;
                        }
                }

                if (physical_memory_page_unreference((uint32_t)phys_page) < 0)
                {
                        printf("Cannot dealloc page\n");
                        return;
                }

                nb_free_physical_pages++;

                vga_set_position(30, 7);
                printf("Can free %d pages\n", nb_free_physical_pages);
        }

        assert(nb_allocated_physical_pages == nb_free_physical_pages);

	printf("Can allocate %d bytes and free %d bytes \n", 
		nb_allocated_physical_pages << X86_PAGE_SHIFT,
		nb_free_physical_pages << X86_PAGE_SHIFT);
}
