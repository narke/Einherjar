/**
 * @author Konstantin Tcholokachvili
 * @date 2013
 * @license MIT License
 * 
 * Roentgenium's kernel's main file
 */

#include <arch/all/status.h>
#include <arch/x86-pc/input_output/screen/vga.h>
#include <arch/x86-all/mmu/gdt.h>
#include <arch/x86-all/interrupts/idt.h>
#include <arch/x86-all/interrupts/isr.h>
#include <arch/x86-all/interrupts/irq.h>
#include <arch/x86-pc/timer/pit.h>
#include <arch/x86-pc/input_output/keyboard/keyboard.h>
#include <arch/all/klibc.h>
#include <arch/x86-pc/bootstrap/multiboot.h>
#include <memory_manager/physical_memory.h>
#include <arch/x86-all/mmu/paging.h>
#include <memory_manager/virtual_memory.h>
#include <arch/x86-all/process/tss.h>
#include <arch/x86-all/process/syscalls.h>


/**
 * The kernel entry point. All starts from here!
 */

void roentgenium_main(uint32_t magic, uint32_t address)
{
    uint16_t retval;
    multiboot_info_t *mbi;
    mbi = (multiboot_info_t *)address;
    
    // Memory manager variables
    paddr_t physical_addresses_bottom;
    paddr_t physical_addresses_top;
    uint32_t ram_size;

    // RAMFS
    uint32_t ramfs_start = 0;
    uint32_t ramfs_end = 0;

    // VGA scren setup
    vga_clear();
    vga_set_attributes(FG_BRIGHT_BLUE | BG_BLACK );
    vga_set_position(34, 0);
	
    printf("Roentgenium\n");

    // RAM size
    ram_size = (unsigned int)mbi->mem_upper;

    printf("RAM is %dMB (upper mem = %x kB)\n", 
		(ram_size >> 10) + 1, ram_size);

    // GDT
    x86_gdt_setup();
	
    printf("CPU: GDT");
	
    // IDT
    x86_idt_setup();
	
    printf(" | IDT");

    // ISRs: Exceptions + System call handler
    x86_isr_setup();

    printf(" | Exceptions + System call");

    // IRQs
    x86_irq_setup();

    printf(" | IRQs\n");

    // Timer: Raise IRQ0 at 100 Hz rate
    retval = x86_pit_set_frequency(100);

    assert(retval == KERNEL_OK);

    // Timer interrupt, momentarily disabled
    x86_irq_set_handler(IRQ_TIMER, timer_interrupt_handler);

    // Keyboard interrupt
    x86_irq_set_handler(IRQ_KEYBOARD, keyboard_interrupt_handler);

    printf("Modules: %d \n", mbi->mods_count);

    ramfs_start = *((uint32_t*)mbi->mods_addr);
    ramfs_end = *(uint32_t*)(mbi->mods_addr + 4);

    // Memory management: Physical memory management
    retval = physical_memory_setup((mbi->mem_upper<<10) + (1<<20),
		&physical_addresses_bottom,
		&physical_addresses_top,
		ramfs_start,
		ramfs_end);

    assert(retval == KERNEL_OK);

    printf("Memory Manager: Physical memory");

    // Memory management: Paging
    retval = x86_paging_setup(physical_addresses_bottom,
		              physical_addresses_top);

    assert(retval == KERNEL_OK);

    printf(" | Paging");

    // Memory Management: Virtual memory
    vmm_setup(physical_addresses_bottom,
              physical_addresses_top);

    printf(" | Virtual memory\n");

    // Enable interrupts
    __asm__ __volatile__ ("sti");


    x86_tss_setup();
    syscalls_setup();


    // Trying to go in user mode
#define USTACK_NPAGES 8
    
    int i, stack_top_uaddr;
    int nb_threads = 1;

    for (i = 0, stack_top_uaddr = 0xfffffffc ;
        i < nb_threads ;
        i++, stack_top_uaddr -= (USTACK_NPAGES + 4)*X86_PAGE_SIZE)
    {
	int p;
	unsigned int stack_base = PAGE_ALIGN_LOWER_ADDRESS(stack_top_uaddr);

	for (p = 0 ; p < USTACK_NPAGES ; p++, stack_base -= X86_PAGE_SIZE)
	{
		uint32_t page_physical_address;
		uint32_t retval;

		page_physical_address = physical_memory_page_reference_new();
		assert(page_physical_address != (uint32_t)NULL);

		retval = x86_paging_map(page_physical_address, 
					stack_base,
					TRUE,
					VM_FLAG_READ | VM_FLAG_WRITE);

		assert(retval == KERNEL_OK);

		retval = physical_memory_page_unreference(page_physical_address);
		assert(retval == 0);
	}
    }

    x86_tss_set_kernel_stack(stack_top_uaddr);


    asm volatile("  \
     	cli; \
     	mov $0x23, %ax; \
     	mov %ax, %ds; \
     	mov %ax, %es; \
     	mov %ax, %fs; \
     	mov %ax, %gs; \
                   \
     	mov %esp, %eax; \
     	pushl $0x23; \
     	pushl %eax; \
     	pushf; \
     	pop %eax;\
     	orl $0x200, %eax;\
     	push %eax;\
     	pushl $0x1B;\
     	push $1f;\
     	iret; \
   	1: \
     	");

    syscall_vga_display_string("Userland function executed via a system call\n");
}
