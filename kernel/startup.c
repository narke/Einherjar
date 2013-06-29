/**
 * @file startup.c
 * @author Konstantin Tcholokachvili
 * @date 2013
 * Roentgenium's kernel's main file
 */

#define MULTIBOOT_BOOTLOADER_MAGIC 0x2BADB002

#include <libraries/return_values.h>
#include <hardware/input_output/screen/vga.h>
#include <hardware/cpu/x86/mmu/gdt.h>
#include <hardware/cpu/x86/interrupts/idt.h>
#include <hardware/cpu/x86/interrupts/isr.h>
#include <hardware/cpu/x86/interrupts/irq.h>
#include <hardware/timer/pit.h>
#include <hardware/input_output/keyboard/keyboard.h>

/**
 * The kernel entry point. All starts from here!
 */

void roentgenium_main(void)
{
    extern unsigned int magic;

    uint16_t ret;
 
    if ( magic != MULTIBOOT_BOOTLOADER_MAGIC )
    {
        ; /* Error */
    }
    
    // VGA scren setup
    vga_clear();
    vga_set_attributes(FG_BRIGHT_BLUE | BG_BLACK );
    vga_set_position(34, 0);
	
    vga_display_string("Roentgenium\n");
	
    // GDT
    x86_gdt_setup();
	
    vga_display_string("CPU: GDT");
	
    // IDT
    x86_idt_setup();
	
    vga_display_string(" | IDT");

    // Exceptions
    x86_exceptions_setup();

    vga_display_string(" | Exceptions");

    // IRQs
    x86_irq_setup();

    vga_display_string(" | IRQs");

    // Timer: Raise IRQ0 at 100 Hz rate
    ret = x86_pit_set_frequency(100);

    if ( ret != KERNEL_OK )
    {
	vga_display_string("Kernel Panic: PIT\n");
	return;
    }

    // Timer interrupt, momentarily disabled
    //x86_irq_set_handler(IRQ_TIMER, timer_interrupt_handler);

    // Keyboard interrupt
    x86_irq_set_handler(IRQ_KEYBOARD, keyboard_interrupt_handler);

    // Enable interrupts
    __asm__ __volatile__ ("sti");
}
