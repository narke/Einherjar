# @see http://wiki.osdev.org/Bare_Bones
# Slightly modified by me

.global multiboot_entry

# setting up the Multiboot header - see GRUB docs for details
.set ALIGN,    1<<0                     # align loaded modules on page boundaries
.set MEMINFO,  1<<1                     # provide memory map
.set FLAGS,    ALIGN | MEMINFO          # this is the Multiboot 'flag' field
.set MAGIC,    0x1BADB002               # 'magic number' lets bootloader find the header
.set CHECKSUM, -(MAGIC + FLAGS)         # checksum required
.set STACK_SIZE, 0x4000			# our stack size is 16KiB


# The multiboot header must come first.
.section ".multiboot"

# Multiboot header must be aligned on a 4-byte boundary
.align 4

multiboot_header:
.long MAGIC
.long FLAGS
.long -(MAGIC + FLAGS)

# The beginning of our kernel code
.text


multiboot_entry:
	movl $(stack + STACK_SIZE), %esp	# set up the stack
	movl  %eax, magic			# Multiboot magic number
	movl  %ebx, mbd				# Multiboot data structure

	call roentgenium_main			# calling the kernel

hang:
	hlt					# something bad happened, machine halted
	jmp hang



# reserve initial kernel stack space
stack:
.skip STACK_SIZE	# reserve 16 KiB stack
.comm  mbd, 4		# we will use this in kernel's main
.comm  magic, 4		# we will use this in kernel's main
