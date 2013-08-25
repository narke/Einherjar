#include <arch/x86-all/mmu/segment.h>
#include <arch/x86-all/mmu/gdt.h>
#include <arch/all/klibc.h>

#include "tss.h"


struct x86_tss
{
	uint16_t back_link;
	uint16_t reserved1;

	vaddr_t esp0;

	uint16_t ss0;
	uint16_t reserved2;

	vaddr_t esp1;

	uint16_t ss1;
	uint16_t reserved3;

	vaddr_t esp2;

	uint16_t ss2;
	uint16_t reserved4;

	vaddr_t cr3;
	vaddr_t eip;
	uint32_t eflags;
	uint32_t eax;
	uint32_t ecx;
	uint32_t edx;
	uint32_t ebx;
	uint32_t esp;
	uint32_t ebp;
	uint32_t esi;
	uint32_t edi;

	uint16_t es;
	uint16_t reserved5;

	uint16_t cs;
	uint16_t reserved6;

	uint16_t ss;
	uint16_t reserved7;

	uint16_t ds;
	uint16_t reserved8;

	uint16_t fs;
	uint16_t reserved9;

	uint16_t gs;
	uint16_t reserved10;

	uint16_t ldtr;
	uint16_t reserved11;

	uint16_t debug_trap_flag :1;
	uint16_t reserved12      :15;
	uint16_t iomap_base_address;

} __attribute__((packed));


static struct x86_tss kernel_tss;

void x86_tss_setup()
{
	memset(&kernel_tss, 0x0, sizeof(kernel_tss));

	/* According to Intel (section 6.4.1 of Intel x86 vol 1)
	 * only a correct value of "ss0" and "esp0" fields are required.
	 * Since "esp0" will be updated at priviledge change time
	 * we will not set it up now.
	 */
	kernel_tss.ss0 = X86_BUILD_SEGMENT_REGISTER_VALUE(DPL0, KERNEL_DATA_SEGMENT);
	
	x86_gdt_kernel_tss_setup((vaddr_t)&kernel_tss);
}

void x86_tss_set_kernel_stack(uint32_t stack)
{
        kernel_tss.esp0 = stack;
}

