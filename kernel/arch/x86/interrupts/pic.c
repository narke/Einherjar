#include <arch/x86/io-ports.h>
#include <lib/status.h>

#include "pic.h"

#define PIC_MASTER_COMMAND 0x20
#define PIC_SLAVE_COMMAND  0xa0


void x86_pic_setup(void)
{
	/* Send ICW1: 8086 mode + NOT Single ctrl + call address interval=8 */
	outb(PIC_MASTER_COMMAND, 0x11);
	outb(PIC_SLAVE_COMMAND, 0x11);
	
	/* Send ICW2: ctrl base address */
	outb(PIC_MASTER_COMMAND+1, 0x20);
	outb(PIC_SLAVE_COMMAND+1, 0x28);
	
	/* Send ICW3 master: mask where slaves are connected */
	outb(PIC_MASTER_COMMAND+1, 0x4);
	/* Send ICW3 slave: index where the slave is connected on master */
	outb(PIC_SLAVE_COMMAND+1, 0x2);
	
	/* Send ICW4: 8086 mode, fully nested, not buffered, no implicit EOI */
	outb(PIC_MASTER_COMMAND+1, 0x1);
	outb(PIC_SLAVE_COMMAND+1, 0x1);
	
	/* Send OCW1:
	 * Closing all IRQs : waiting for a correct handler The only IRQ
	 * enabled is the cascade (that's why we use 0xFB for the master) */
	outb(PIC_MASTER_COMMAND+1, 0xFB);
	outb(PIC_SLAVE_COMMAND+1, 0xFF);
}


void x86_pic_enable_irq_line(uint32_t param_irq_number)
{
	if( param_irq_number < 8 ) /*  IRQ on master PIC */
		outb(PIC_MASTER_COMMAND+1, 
				(inb(PIC_MASTER_COMMAND+1) & ~(1 << param_irq_number)));
	else /*  IRQ on slave PIC */
		outb(PIC_SLAVE_COMMAND+1,
				(inb(PIC_SLAVE_COMMAND+1) & ~(1 << (param_irq_number-8))));
}


void x86_pic_disable_irq_line(uint32_t param_irq_number)
{
	if( param_irq_number < 8 ) /*  irq on master PIC */
		outb((inb(PIC_MASTER_COMMAND+1) | (1 << param_irq_number)),
				PIC_MASTER_COMMAND+1);
	else /*  irq on slave PIC */
		outb((inb(PIC_SLAVE_COMMAND+1) | (1 << (param_irq_number-8))),
				PIC_SLAVE_COMMAND+1);
}
