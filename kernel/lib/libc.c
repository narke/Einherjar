/**
* @author Konstantin Tcholokachvili
* @date 2013, 2016
* @license MIT License
*
* Tiny C library for the kernel land
*/

#include <arch/x86-pc/io/vga.h>
#include <memory/physical-memory.h>

#include "libc.h"

bool_t is_number(char *ptr)
{
	while (*ptr)
	{
		switch(*ptr++)
		{
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			case '\0':
				break;
			default:
				return FALSE;
		}
	}

	return TRUE;
}

int atoi(const char *ptr)
{
	int k = 0;
	while (*ptr)
	{
		k = k * 10 + (*ptr) - '0';
		ptr++;
	}

	return k;
}



static void reverse(char str[], int length)
{
	char tmp;
	int i = 0;
	int j = 0;

	for (i = length - 1, j = 0; j < i; i--, j++)
	{
		tmp = str[j];
		str[j] = str[i];
		str[i] = tmp;
	}
}

static char *itoa(int value, char *str, int base)
{
	int remainder;
	int i		= 0;
	int is_negative = 0;

	// Convert 0
	if (value == 0)
	{
		str[i++] = '0';
		str[i]   = '\0';
		return str;
	}

	// Handle negative decimal values
	if (value < 0 && base == 10)
	{
		is_negative = 1;
		value       = -value;
	}

	// Convert the value into the corresponding base
	while (value != 0)
	{
		remainder = value % base;
		str[i++]  = (remainder > 9) ? (remainder-10) + 'a' : remainder + '0';
		value     = value / base;
	}

	// Add '-' to negative numbers now, to reverse that later
	if (is_negative)
		str[i++] = '-';

	// Finalizing by ending the string and reversing it
	str[i] = '\0';

	reverse(str, i);

	return str;
}


void printf(const char *format, ...)
{
	uint8_t c;
	int base;
	char buffer[20];
	char *ptr_str;

	char **arg = (char **)&format;

	arg++;

	while ((c = *format++) != '\0')
	{
		if (c != '%')
		{
			vga_display_character(c);
			continue;
		}

		c = *format++;

		switch (c)
		{
			case '%':
				vga_display_character('%');
				break;

			case 'i':
			case 'd':
			case 'x':
				if (c == 'i' || c == 'd')
					base = 10;
				else
					base = 16;

				itoa(*((int *)arg++), buffer, base);
				ptr_str = buffer;
				goto string;
				break;

			case 's':
				ptr_str = *arg++;
				if (!ptr_str)
					ptr_str = "(null)";

				string:
					while (*ptr_str)
						vga_display_character(*ptr_str++);
				break;

			default:
				vga_display_character(*((int *)arg++));
		}
	}
}

void *memset(void *dst, int c, size_t length)
{
	char *p;

	for (p = (char *)dst; length > 0; p++, length--)
		*p = (char)c;

	return p;
}

void *memcpy(void *dst, const void *src, register size_t size)
{
	char *_dst;
	const char *_src;

	for (_dst = (char*)dst, _src = (const char*)src;
			size > 0 ;
			_dst++, _src++, size--)
		*_dst = *_src;

	return dst;
}

char *strzcpy(register char *dst, register const char *src, register size_t len)
{
	size_t i;

	if (len <= 0)
		return dst;

	for (i = 0; i < len; i++)
	{
		dst[i] = src[i];

		if(src[i] == '\0')
			return dst;
	}

	dst[len-1] = '\0';
	return dst;
}

char *
strchr(const char *s, int c)
{
	while (*s++)
	{
		if (*s == (char)c)
			return (char *)s;
	}

	return NULL;
}

size_t
strlen(const char *s)
{
	uint32_t length = 0;

	while (*s++)
		length++;

	return length;
}

void *malloc(size_t size)
{
	return heap_alloc(size);
}

void free(void *ptr)
{
	heap_free(ptr);
}
