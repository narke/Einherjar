/**
 * @author Konstantin Tcholokachvili
 * @date 2013
 * @license MIT License
 * 
 * Programmable Interrupt Timer
 */


#include <lib/status.h>
#include <arch/x86/io-ports.h>
#include <arch/x86/interrupts/irq.h>
#include <threading/scheduler.h>

#include "pit.h"

/** 82C54's clock's maximal frequency */
#define MAX_FREQUENCY 1193180

/* 82C54's ports */
#define CHANNEL0  0x40	/* Raise an IRQ periodically */
#define CHANNEL1  0x41	/* DRAM refresh */
#define CHANNEL2  0x42	/* PC speaker */
#define CONTROL_REGISTER 0x43



/**
 * "The timer will divide it's input clock of 1.19MHz (1193180Hz)
 * by the number you give it in the data register to figure out
 * how many times per second to fire the signal for that channel."
 * Brandon Friesen
 *
 * IRQ0 will be raised periodically.
 */
ret_t x86_pit_set_frequency(uint32_t frequency)
{
	uint32_t divisor;
	
	if (frequency <= 0 || frequency > MAX_FREQUENCY)
		return -KERNEL_INVALID_VALUE;

	/* Ticks per second
	 * The divisor (16 bits value) is the value which divides the PIT's
	 * input clock (1193180 Hz) to get the desired frequency */
	divisor = MAX_FREQUENCY / frequency;

	/* The divisor must be between 1 and 65536 (16 bits value) */
	if (divisor <= 0 || divisor > 65536)
		return -KERNEL_INVALID_VALUE;

	/* Prevent the divisor value to overflow, because it is coded
	 * on 16 bits (can't be greater than 65536) */
	if (divisor == 65536)
		divisor = 0;

	/* Configure the timer (channel 0) frequency by sending LSB+MSB
	 * (0x30) and tell it to behave as a rate generator (mode 2: 0x4)
	 * 0x30 + 0x4 = 0x34 */
	outb(CONTROL_REGISTER, 0x34);

	/* Send LSB */
	outb(CHANNEL0, (divisor & 0xFF));

	/* Send MSB */
	outb(CHANNEL0, (divisor >> 8) & 0xFF);

	return KERNEL_OK;
}

void timer_interrupt_handler(int number)
{
    static int ticks = 0;
    static int seconds = 0;
    uint32_t flags;

    (void)number; // Avoid a useless warning ;-)

    ticks++;

    if (ticks % 100 == 0)
    {
        seconds++;
        ticks = 0;
    }

    X86_IRQs_DISABLE(flags);
    schedule();
    X86_IRQs_ENABLE(flags);
}


