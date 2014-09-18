#include <lib/libc.h>

#include "initrd-test.h"


void initrd_test(uint32_t initrd_start,
		uint32_t initrd_end)
{
	printf("\nInitrd block contents:\n");

	printf("%s", initrd_start);
}

