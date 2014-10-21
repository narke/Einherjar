#include <lib/libc.h>
#include <arch/x86-all/interrupts/irq.h>
#include <threading/thread.h>

#include "threading-test.h"

void print_hello(void *a)
{
	printf("Hello ");
}

void print_world(void *a)
{
	printf("world!\n");
}

void test_kernel_threads(void)
{
	uint32_t flags;

	X86_IRQs_DISABLE(flags);
	
	thread_create("h", print_hello, 0);
	thread_create("w", print_world, 0);
	
	X86_IRQs_ENABLE(flags);
}


