#ifndef _IO_PORTS_H_
#define _IO_PORTS_H_


/**	
 * @file io_ports.h
 * @author Konstantin Tcholokachvili
 * @@date 2007
 * @license MIT License
 * 
 * x86 I/O space access functions.
 */


/** 
	Write a value to an I/O port 
	@param value
	@param port
	@return None
*/
#define outb(port, value)		\
  __asm__ __volatile__ (		\
        "outb %b0,%w1"			\
        ::"a" (value),"Nd" (port)	\
        )				\

/** 
	Read one byte from I/O port
	@param port
	@return value
*/
#define inb(port)		\
({				\
  unsigned char _value;		\
  __asm__ __volatile__ (	\
        "inb %w1,%0"		\
        :"=a" (_value)		\
        :"Nd" (port)		\
        );			\
  _value;			\
})

	
#endif // _IO_PORTS_H_ 
