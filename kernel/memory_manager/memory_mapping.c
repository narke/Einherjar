#include <memory_manager/virtual_memory.h>
#include <arch/all/klibc.h>

#include "memory_mapping.h"


void *mmap(void *address, size_t length, int protection, int flags,
	   int fd, off_t offset)
{
	void *ptr;

	vm_access_t vm_flags = 0;

	if (protection & PROT_READ)
		vm_flags |= VM_R;
	if (protection & PROT_WRITE)
		vm_flags |= VM_W;
	if (protection & PROT_EXEC)
		vm_flags |= VM_X;

	ptr = heap_alloc(length, vm_flags);
	
	if (!ptr)
		return MAP_FAILED;

	memset(ptr, 0, length);

	return ptr;
}

int munmap(void *address, size_t length)
{
	heap_free(address);

	return 0;
}

