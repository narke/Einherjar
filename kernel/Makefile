CC      = gcc
PWD    := $(shell pwd)
CFLAGS  = -Wall -m32 -Wa,--32 -nostdlib -nostdinc -ffreestanding -I$(PWD)
LDFLAGS = --warn-common -melf_i386 --strip-all

BOOTLOADER_PATH = ../bootloader
BUILD_PATH      = ../build

OBJECTS = $(BOOTLOADER_PATH)/multiboot.o	\
	hardware/input_output/screen/vga.o	\
	hardware/cpu/x86/mmu/gdt.o 		\
	hardware/cpu/x86/interrupts/idt.o 	\
	hardware/cpu/x86/interrupts/isr_stubs.o	\
	hardware/cpu/x86/interrupts/isr.o	\
	hardware/cpu/x86/interrupts/irq_stubs.o	\
	hardware/cpu/x86/interrupts/irq.o	\
	hardware/timer/pit.o			\
	startup.o

KERNEL   = $(BUILD_PATH)/roentgenium.elf
MULTIBOOT_IMAGE = $(BUILD_PATH)/roentgenium.iso

all: kernel cdrom

kernel: $(KERNEL)

$(KERNEL): $(OBJECTS)
	@if [ ! -d $(BUILD_PATH) ]; 	\
	then 				\
		mkdir $(BUILD_PATH); 	\
	fi
	$(LD) $(LDFLAGS) -T ./linker.ld -o $@ $^

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

%.o: %.S
	$(CC) -I$(PWD) -c $< $(CFLAGS) -o $@

%.o: %.asm
	nasm -f elf $<

cdrom: 
	mv $(KERNEL) $(BOOTLOADER_PATH)/grub/
	mkisofs -R -b boot/grub/stage2_eltorito -no-emul-boot \
	-boot-load-size 4 -boot-info-table -o $(MULTIBOOT_IMAGE) $(BOOTLOADER_PATH)/grub/

clean:
	$(RM) $(BUILD_PATH)/*.iso
	$(RM) $(BOOTLOADER_PATH)/*.o
	$(RM) $(BOOTLOADER_PATH)/grub/roentgenium.elf
	$(RM) *.o
	$(RM) hardware/input_output/screen/*.o
	$(RM) hardware/cpu/x86/mmu/*.o
	$(RM) hardware/cpu/x86/interrupts/*.o
	$(RM) hardware/timer/*.o
	@if [ -d $(BUILD_PATH) ]; 		\
	then 					\
		rm -R $(BUILD_PATH); 		\
		echo "rm -R $(BUILD_PATH)" ;	\
	fi
