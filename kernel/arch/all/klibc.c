/**
* @author Konstantin Tcholokachvili
* @date 2013
* @license MIT License
*
* Tiny C library for the kernel land
*/

#include <arch/x86-pc/input_output/screen/vga.h>
#include <memory_manager/memory_mapping.h>

#include "klibc.h"

static void itoa(unsigned int value, char *str, int base)
{
	uint8_t i = 0;
	uint8_t j = 0;
	uint8_t divisor;
	uint8_t remainder;
	char tmp;
	
	if (value == 0)
	{
		str[i++] = '0';
	}
		
	switch (base)
	{
		case 'd':
			divisor = 10;
			
			if (value < 0)
			{
				str[i++] = '-';
				value = -value;
			}
			break;
		
	
		case 'x':
			divisor = 16;
			break;
			
		default:
			divisor = 10;
	}	

	while (value > 0)
	{
		remainder = value % divisor;
		str[i++] = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
		value = value / divisor;
	}
	
	if (base == 'x')
	{
		// Display 0xN
		str[i++] = 'x';
		str[i++] = '0'; 
	}

	str[i] = '\0';
	
	// reversing the array to get the numbers in order
	for (i = i - 1, j = 0; j < i; i--, j++)
	{
		tmp = str[j];
		str[j] = str[i];
		str[i] = tmp;
	}
}


void printf(const char *format, ...)
{
	uint8_t c;
	char buffer[20];

	char **arg = (char **)&format;

	arg++;

	while ( (c = *format++) != '\0')
	{	
		if (c != '%')
		{
			vga_display_character(c);
		}
		else
		{
			char *ptr_str;
			
			c = *format++;
	
			switch (c)
			{
				case '%':
					vga_display_character('%');
					break;

				case 'd':
				case 'x':
					itoa(*((int *)arg++), buffer, c);
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
}


void *memset(void *dst, sint32_t c, uint32_t length)
{
	char *p;

	for (p = (char *)dst; length > 0; p++, length--)
	{
		*p = (char)c;
	}
	
	return p;
}


void *memcpy(void *dst, const void *src, size_t n)
{
	char *dst8 = (char *)dst;
	const char *src8 = (char *)src;
	
	while (n--)
	{
		*dst8++ = *src8++;
	}

	return dst;
}


void *malloc(size_t size)
{
        void *ptr = dlmalloc(size);
        return ptr;
}


void free(void *ptr)
{
	dlfree(ptr);
}
