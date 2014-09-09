/**
 * @author Konstantin Tcholokachvili
 * @date 2013, 2014
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
#include <process/thread.h>
#include <process/scheduler.h>
#include <test_suite/initrd_test.h>


static void idle_thread()
{
    while (1)
    {
        asm("hlt\n");
    }
}


/**
 * The kernel entry point. All starts from here!
 */
void roentgenium_main(uint32_t magic, uint32_t address)
{
    uint16_t retval;
    multiboot_info_t *mbi;
    mbi = (multiboot_info_t *)address;

    extern unsigned int x86_kernel_stack_bottom;
    extern unsigned int x86_kernel_stack_size;
    
    // Memory manager variables
    paddr_t physical_addresses_bottom;
    paddr_t physical_addresses_top;
    uint32_t ram_size;

    // Initrd
    uint32_t initrd_start = 0;
    uint32_t initrd_end = 0;

    // VGA scren setup
    vga_clear();
    vga_set_attributes(FG_BRIGHT_BLUE | BG_BLACK );
    vga_set_position(34, 0);
	
    printf("Roentgenium\n");

    // RAM size
    ram_size = (unsigned int)mbi->mem_upper;

    printf("RAM is %dMB\n", (ram_size >> 10) + 1);

    // GDT
    x86_gdt_setup();
	
    printf("CPU: GDT");
	
    // IDT
    x86_idt_setup();
	
    printf(" | IDT");

    // ISRs: Exceptions
    x86_isr_setup();

    printf(" | Exceptions");

    // IRQs
    x86_irq_setup();

    printf(" | IRQs\n");

    // Timer: Raise IRQ0 at 100 Hz rate
    retval = x86_pit_set_frequency(100);

    assert(retval == KERNEL_OK);

    // Timer interrupt, momentarily disabled
    x86_irq_set_routine(IRQ_TIMER, timer_interrupt_handler);

    // Keyboard interrupt
    x86_irq_set_routine(IRQ_KEYBOARD, keyboard_interrupt_handler);

	// Initrd: Initial Ram Disk
	initrd_start = *((uint32_t *)mbi->mods_addr);
	initrd_end   = *(uint32_t *)(mbi->mods_addr + 4);

    // Memory management: Physical memory management
    retval = physical_memory_setup((mbi->mem_upper<<10) + (1<<20),
			&physical_addresses_bottom,
			&physical_addresses_top,
			initrd_start,
			initrd_end);

    assert(retval == KERNEL_OK);

    printf("Memory Manager: Physical memory");

    // Memory management: Paging
    retval = x86_paging_setup(physical_addresses_bottom,
			physical_addresses_top);

    assert(retval == KERNEL_OK);

    printf(" | Paging");

    // Memory Management: Virtual memory
    virtual_memory_setup(physical_addresses_bottom,
			physical_addresses_top,
			ram_size);

    printf(" | Virtual memory\n");

    // Kernel threads
    retval = threading_setup(x86_kernel_stack_bottom,
			x86_kernel_stack_bottom	+ x86_kernel_stack_size);

    assert(retval == KERNEL_OK);

    printf("Kernel threads\n");

    // Scheduler
    scheduler_setup();

    // Declare the idle thread
    assert(thread_create("idle", idle_thread, NULL) != NULL);

    // Enable interrupts
    asm volatile("sti");

	printf("Initrd\n");

	initrd_test(initrd_start, initrd_end);
}
