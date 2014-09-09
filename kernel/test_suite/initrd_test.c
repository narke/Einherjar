#include <arch/all/klibc.h>

#include "initrd_test.h"


void initrd_test(uint32_t initrd_start,
		uint32_t initrd_end)
{
	printf("\nInitrd block contents:\n");

	printf("%s", initrd_start);
}

