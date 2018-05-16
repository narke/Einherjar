VERBOSE = off # Set this to see commands being run
COLOR   = on

include messages.make

CC      = gcc
PWD    := $(shell pwd)
CFLAGS  = -Wall -Wextra -m32 -Wa,--32 -nostdlib -nostdinc -ffreestanding -fgnu89-inline -I$(PWD)
LDFLAGS = --warn-common -melf_i386 --strip-all

BOOTLOADER_PATH = arch/x86-pc/bootstrap
BUILD_PATH      = ../build
INITRD_PATH     = arch/x86-pc/bootstrap/iso

OBJECTS = $(BOOTLOADER_PATH)/multiboot.o        \
	arch/x86-pc/io/vga.o                    \
	arch/x86/mmu/gdt.o                      \
	arch/x86/interrupts/idt.o               \
	arch/x86/interrupts/isr-stubs.o         \
	arch/x86/interrupts/isr.o               \
	arch/x86/interrupts/pic.o               \
	arch/x86/interrupts/irq-stubs.o         \
	arch/x86/interrupts/irq.o               \
	arch/x86-pc/timer/pit.o                 \
	arch/x86-pc/io/keyboard.o               \
	lib/libc.o                              \
	memory/physical-memory.o                \
	arch/x86/threading/cpu-context.o        \
	arch/x86/threading/cpu-context-switch.o \
	threading/thread.o                      \
	threading/scheduler.o                   \
	io/console.o                            \
	colorforth/editor.o                     \
	colorforth/compiler.o                   \
	arch/x86-pc/startup.o

KERNEL          = $(BUILD_PATH)/roentgenium.elf
MULTIBOOT_IMAGE	= $(BUILD_PATH)/roentgenium.iso

all: kernel initrd cdrom

kernel: $(KERNEL)

$(KERNEL): $(OBJECTS)
	@if [ ! -d $(BUILD_PATH) ];  \
	then                         \
		mkdir $(BUILD_PATH); \
	fi
	$(linking) '$< > $@'
	$(LD) $(LDFLAGS) -T arch/x86-pc/linker.ld -o $@ $^

%.o: %.c
	$(compiling) '$< > $@'
	$(CC) -c $< -o $@ $(CFLAGS)

%.o: %.asm
	$(assembling) '$< > $@'
	nasm -f elf $<

cdrom:
	$(generating) '$@'
	mv $(KERNEL) $(BOOTLOADER_PATH)/iso/boot/
	grub-mkrescue -o $(MULTIBOOT_IMAGE) $(BOOTLOADER_PATH)/iso/

initrd:
	$(generating) '$@'
	../tools/blocks_converter.py tocf ../initrd/blocks.cfs $(INITRD_PATH)/initrd.img

run:
	qemu-system-i386 -m 16 -cdrom $(MULTIBOOT_IMAGE)

clean:
	$(cleaning)
	@if [ -d $(BUILD_PATH) ];    \
	then                         \
		rm -R $(BUILD_PATH); \
	fi
	$(RM) $(OBJECTS)
	$(RM) $(BOOTLOADER_PATH)/iso/boot/roentgenium.elf
	$(RM) $(INITRD_PATH)/initrd.img
