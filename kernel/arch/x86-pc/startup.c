/**
 * @author Konstantin Tcholokachvili
 * @date 2013, 2014, 2017
 * @license MIT License
 *
 * Einherjar's kernel's main file
 */

#include <lib/status.h>
#include <arch/x86-pc/io/vga.h>
#include <arch/x86/mmu/gdt.h>
#include <arch/x86/interrupts/idt.h>
#include <arch/x86/interrupts/isr.h>
#include <arch/x86/interrupts/irq.h>
#include <arch/x86-pc/timer/pit.h>
#include <lib/libc.h>
#include <arch/x86-pc/bootstrap/multiboot.h>
#include <memory/physical-memory.h>
#include <threading/thread.h>
#include <threading/scheduler.h>
#include <io/console.h>
#include <colorforth/colorforth.h>


/**
 * The kernel entry point. All starts from here!
 */
void roentgenium_main(uint32_t magic, uint32_t address)
{
    uint16_t retval;
    multiboot_info_t *mbi;
    mbi = (multiboot_info_t *)address;

    (void)magic; // Avoid a useless warning ;-)

    // Initrd
    uint32_t initrd_start;
    uint32_t initrd_end;

    // Console
    struct console *cons = NULL;

    // GDT
    x86_gdt_setup();

    // IDT
    x86_idt_setup();

    // ISRs: Exceptions
    x86_isr_setup();

    // IRQs
    x86_irq_setup();

    // Timer: Raise IRQ0 at 100 Hz rate
    retval = x86_pit_set_frequency(100);

    assert(retval == KERNEL_OK);

    // Timer interrupt
    x86_irq_set_routine(IRQ_TIMER, timer_interrupt_handler);

    // Initrd: Initial Ram Disk
    initrd_start = *((uint32_t *)mbi->mods_addr);
    initrd_end   = *(uint32_t *)(mbi->mods_addr + 4);

    // Physical memory management
    physical_memory_setup((mbi->mem_upper<<10) + (1<<20),
		    initrd_start,
		    initrd_end);

    // Kernel threads
    threading_setup();

    // Scheduler
    scheduler_setup();

    // Enable interrupts
    asm volatile("sti");

    // Console
    console_setup(&cons, vga_display_character);

    // colorForth
    colorforth_initialize();

    struct editor_args *args = malloc(sizeof(struct editor_args));

    args->cons = cons;
    args->initrd_start = initrd_start;
    args->initrd_end = initrd_end;

    thread_create("editor", editor, args);
}
