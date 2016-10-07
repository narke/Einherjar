#include <lib/status.h>

#include "idt.h"
#include "pic.h"
#include "irq.h"


#define X86_IRQ_NUM	16

/* array of IRQ wrappers, defined in irq_wrappers.S */
extern uint32_t x86_irq_wrapper_array[X86_IRQ_NUM];

/* arrays of IRQ handlers, shared with irq_wrappers.S */
x86_irq_handler_t x86_irq_handler_array[X86_IRQ_NUM] = { NULL, };

void x86_irq_setup(void)
{
	x86_pic_setup();
}


ret_t x86_irq_set_routine(uint32_t irq_level, x86_irq_handler_t routine)
{
	uint16_t ret;
	uint32_t flags;

	if (irq_level >= X86_IRQ_NUM)
		return -KERNEL_INVALID_VALUE;

	X86_IRQs_DISABLE(flags);

	/* Set the irq routine to be called by the IRQ wrapper */
	x86_irq_handler_array[irq_level] = routine;

	/* If the irq is to be enabled, update the IDT with the IRQ  wrapper */
	if (routine != NULL)
	{
		ret = x86_idt_set_handler(X86_IRQ_BASE + irq_level,
				(uint32_t) x86_irq_wrapper_array[irq_level]);

		/* A problem occured */
		if (ret != KERNEL_OK)
			x86_irq_handler_array[irq_level] = NULL;
	}
	else /* Disable this idt entry */
	{
		ret = x86_idt_set_handler(X86_IRQ_BASE + irq_level,
				(uint32_t)NULL /* Disable IDTE */);
	}

	/* Update the PIC only if an IRQ handler has been set */
	if (x86_irq_handler_array[irq_level] != NULL)
		x86_pic_enable_irq_line(irq_level);
	else
		x86_pic_disable_irq_line(irq_level);

	X86_IRQs_ENABLE(flags);

	return ret;
}


x86_irq_handler_t irq_get_routine(uint32_t irq_level)
{
	if (irq_level >= X86_IRQ_NUM)
		return NULL;

	/* Expected to be atomic */
	return x86_irq_handler_array[irq_level];
}

