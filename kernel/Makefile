CC      = gcc
PWD    := $(shell pwd)
CFLAGS  = -Wall -m32 -Wa,--32 -nostdlib -nostdinc -ffreestanding -I$(PWD)
LDFLAGS = --warn-common -melf_i386 --strip-all

BOOTLOADER_PATH = arch/x86-pc/bootstrap
BUILD_PATH      = ../build
INITRD_PATH     = arch/x86-pc/bootstrap/iso

OBJECTS = $(BOOTLOADER_PATH)/multiboot.o    \
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
	arch/x86/mmu/paging.o                   \
	memory/virtual-memory.o                 \
	arch/x86/threading/cpu-context.o        \
	arch/x86/threading/cpu-context-switch.o \
	threading/thread.o                      \
	threading/scheduler.o                   \
	test-suite/initrd-test.o                \
	startup.o

KERNEL			= $(BUILD_PATH)/roentgenium.elf
MULTIBOOT_IMAGE	= $(BUILD_PATH)/roentgenium.iso

all: kernel initrd cdrom

kernel: $(KERNEL)

$(KERNEL): $(OBJECTS)
	@if [ ! -d $(BUILD_PATH) ];  \
	then                         \
		mkdir $(BUILD_PATH); \
	fi                           
	$(LD) $(LDFLAGS) -T ./linker.ld -o $@ $^

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

%.o: %.S
	$(CC) -I$(PWD) -c $< $(CFLAGS) -o $@

%.o: %.asm
	nasm -f elf $<

cdrom: 
	mv $(KERNEL) $(BOOTLOADER_PATH)/iso/
	genisoimage -R -b boot/grub/stage2_eltorito -no-emul-boot \
	-boot-load-size 4 -boot-info-table -o $(MULTIBOOT_IMAGE) $(BOOTLOADER_PATH)/iso/

initrd:
	../tools/initrd_generate.py
	mv initrd.img $(INITRD_PATH) 

clean:
	@if [ -d $(BUILD_PATH) ];    \
	then                         \
		rm -R $(BUILD_PATH); \
	fi                        
	$(RM) $(BOOTLOADER_PATH)/*.o
	$(RM) $(BOOTLOADER_PATH)/iso/roentgenium.elf
	$(RM) *.o
	$(RM) arch/x86-pc/io/*.o
	$(RM) arch/x86-all/mmu/*.o
	$(RM) arch/x86-all/interrupts/*.o
	$(RM) arch/x86-pc/timer/*.o
	$(RM) lib/*.o
	$(RM) memory/*.o
	$(RM) arch/x86-all/threading/*.o
	$(RM) threading/*.o
	$(RM) test-suite/*.o
	$(RM) $(INITRD_PATH)/initrd.img
