/**
 * @file startup.c
 * @author Konstantin Tcholokachvili
 * @date 2013
 * Roentgenium's kernel's main file
 */

#define MULTIBOOT_BOOTLOADER_MAGIC 0x2BADB002

#include <arch/all/status.h>
#include <arch/x86-pc/input_output/screen/vga.h>
#include <arch/x86-all/mmu/gdt.h>
#include <arch/x86-all/interrupts/idt.h>
#include <arch/x86-all/interrupts/isr.h>
#include <arch/x86-all/interrupts/irq.h>
#include <arch/x86-pc/timer/pit.h>
#include <arch/x86-pc/input_output/keyboard/keyboard.h>
#include <arch/all/klibc.h>

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
	
    printf("Roentgenium\n");
	
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
    ret = x86_pit_set_frequency(100);

    if ( ret != KERNEL_OK )
    {
	printf("Kernel Panic: PIT\n");
	return;
    }

    // Timer interrupt, momentarily disabled
    //x86_irq_set_handler(IRQ_TIMER, timer_interrupt_handler);

    // Keyboard interrupt
    x86_irq_set_handler(IRQ_KEYBOARD, keyboard_interrupt_handler);

    // Enable interrupts
    __asm__ __volatile__ ("sti");
}
