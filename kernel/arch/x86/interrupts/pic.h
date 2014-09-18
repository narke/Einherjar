#ifndef _PIC_H_
#define _PIC_H_

#include <lib/types.h>
#include "irq.h"

/**
 * @file pic.h Programmable Interruptr Controller
 * @date 2010
 * @license MIT License
 *
 * PIC's functions
 * Only used byinterrupt.c
 *
 * @see Intel 8259A datasheet
 */

/** Setup PIC and Disable all IRQ lines */
void x86_pic_setup(void);

void x86_pic_enable_irq_line(uint32_t numirq);

void x86_pic_disable_irq_line(uint32_t numirq);

#endif
