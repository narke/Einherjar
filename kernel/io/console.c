#include <arch/x86-pc/io/vga.h>
#include <arch/x86-pc/io/keyboard.h>
#include <lib/status.h>
#include <lib/libc.h>
#include <lib/queue.h>

#include "console.h"

#define CONSOLE_BUFFER_LENGTH 64

struct console
{
	char	buffer[CONSOLE_BUFFER_LENGTH];
	uint8_t buffer_read;
	uint8_t buffer_write;
	uint8_t mode;
	void	(*write)(uchar_t c);

	TAILQ_ENTRY(console) next;
};

TAILQ_HEAD(, console) consoles_list;

ret_t console_setup(struct console **terminal_out,
		void (*write_function)(uchar_t c))
{
	struct console *terminal;

	TAILQ_INIT(&consoles_list);

	terminal = malloc(sizeof(struct console));

	if (!terminal)
		return -KERNEL_NO_MEMORY;

	memset(terminal->buffer, 0, sizeof(terminal->buffer));
	terminal->write        = write_function;
	terminal->buffer_read  = 0;
	terminal->buffer_write = 0;
	terminal->mode         = CONSOLE_MODE_CANON;

	TAILQ_INSERT_TAIL(&consoles_list, terminal, next);

	keyboard_setup(terminal);

	*terminal_out = terminal;

	return KERNEL_OK;
}


ret_t console_read(struct console *t,
		uchar_t *dst_buffer,
		size_t len)
{
	size_t count = 0;

	if (len == 0)
		return KERNEL_OK;

	while (1)
	{
		char c;

		/* No character available,  Wait until console_add_character()
		 * wakes us up */
		if (t->buffer_read == t->buffer_write)
		{
			/* Loop, maybe the semaphore was stolen by someone else */
			continue;
		}

		c = t->buffer[t->buffer_read];

		/* Copy the received character from the ring buffer to the
		 * destination buffer */
		memcpy((void *)dst_buffer,
				(void *)&t->buffer[t->buffer_read],
				sizeof(char));

		dst_buffer++;

		/* Update the ring buffer read pointer */
		t->buffer_read++;

		if (t->buffer_read == CONSOLE_BUFFER_LENGTH)
			t->buffer_read = 0;

		count++;

		if (t->mode & CONSOLE_MODE_ECHO)
			t->write(c);

		/* Did we read enough bytes ? */
		if (count == len || (c == '\n' && t->mode & CONSOLE_MODE_CANON))
			break;
	}

	return KERNEL_OK;
}


void console_write(struct console *t, void *src_buffer, uint16_t len)
{
	int i;
	char c;

	for (i = 0; i < len; i++)
	{
		memcpy((void *)&c, (void *)src_buffer, sizeof(char));
		len = i;

		t->write(c);

		src_buffer++;
	}
}


void console_set_mode(struct console *cons, uint8_t mode)
{
	cons->mode = mode;
}

void console_add_character(struct console *cons, char c)
{
	cons->buffer[cons->buffer_write] = c;
	cons->buffer_write++;

	if (cons->buffer_write == CONSOLE_BUFFER_LENGTH)
		cons->buffer_write = 0;

}
