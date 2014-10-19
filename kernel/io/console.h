#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#include <lib/types.h>

#define CONSOLE_MODE_CANON 1
#define CONSOLE_MODE_ECHO  2

struct console;

ret_t console_setup(struct console **terminal_out,
		void (*write_function)(uchar_t c));

ret_t console_read(struct console *t, uchar_t *buffer, size_t len);

void console_write(struct console *t, void *src_buffer, uint16_t len);

void console_add_character(struct console *cons, char c);

void console_set_mode(struct console *cons, uint8_t mode);

#endif // _CONSOLE_H_
