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

/**
 * The kernel entry point. All starts from here!
 */

void roentgenium_main(uint32_t magic, uint32_t address)
{
    uint16_t retval;
    multiboot_info_t *mbi;
    mbi = (multiboot_info_t *)address;
    
    // Memory manager variables
    uint32_t physical_addresses_bottom;
    uint32_t physical_addresses_top;
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

    // Exceptions
    x86_exceptions_setup();

    printf(" | Exceptions");

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

    // Memory manager: physical memory management
    retval = physical_memory_setup((mbi->mem_upper<<10) + (1<<20),
		&physical_addresses_bottom,
		&physical_addresses_top,
		ramfs_start,
		ramfs_end);

    assert(retval == KERNEL_OK);

    printf("Memory Manager: Physical pages management\n");

    // Enable interrupts
    __asm__ __volatile__ ("sti");
}
