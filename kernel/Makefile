CC      = gcc
PWD    := $(shell pwd)
CFLAGS  = -Wall -m32 -Wa,--32 -nostdlib -nostdinc -ffreestanding -I$(PWD)
LDFLAGS = --warn-common -melf_i386 --strip-all

BOOTLOADER_PATH = arch/x86-pc/bootstrap
BUILD_PATH      = ../build

OBJECTS = $(BOOTLOADER_PATH)/multiboot.o			\
	arch/x86-pc/input_output/screen/vga.o			\
	arch/x86-all/mmu/gdt.o							\
	arch/x86-all/interrupts/idt.o					\
	arch/x86-all/interrupts/isr_stubs.o				\
	arch/x86-all/interrupts/isr.o					\
	arch/x86-all/interrupts/pic.o					\
	arch/x86-all/interrupts/irq_stubs.o				\
	arch/x86-all/interrupts/irq.o					\
	arch/x86-pc/timer/pit.o							\
	arch/x86-pc/input_output/keyboard/keyboard.o	\
	arch/all/klibc.o								\
	memory_manager/physical_memory.o				\
	arch/x86-all/mmu/paging.o						\
	memory_manager/virtual_memory.o					\
	arch/x86-all/process/cpu_context.o				\
	arch/x86-all/process/cpu_context_switch.o		\
	process/thread.o								\
	process/scheduler.o								\
	test_suite/threading_test.o						\
	startup.o

KERNEL			= $(BUILD_PATH)/roentgenium.elf
MULTIBOOT_IMAGE	= $(BUILD_PATH)/roentgenium.iso

all: kernel cdrom

kernel: $(KERNEL)

$(KERNEL): $(OBJECTS)
	@if [ ! -d $(BUILD_PATH) ]; \
	then 						\
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
	mv $(KERNEL) $(BOOTLOADER_PATH)/iso/
	genisoimage -R -b boot/grub/stage2_eltorito -no-emul-boot \
	-boot-load-size 4 -boot-info-table -o $(MULTIBOOT_IMAGE) $(BOOTLOADER_PATH)/iso/

clean:
	$(RM) $(BUILD_PATH)/*.iso
	$(RM) $(BOOTLOADER_PATH)/*.o
	$(RM) $(BOOTLOADER_PATH)/iso/roentgenium.elf
	$(RM) *.o
	$(RM) arch/x86-pc/input_output/screen/*.o
	$(RM) arch/x86-all/mmu/*.o
	$(RM) arch/x86-all/interrupts/*.o
	$(RM) arch/x86-pc/timer/*.o
	$(RM) arch/x86-pc/input_output/keyboard/*.o
	$(RM) arch/all/*.o
	$(RM) memory_manager/*.o
	$(RM) arch/x86-all/process/*.o
	$(RM) process/*.o
	$(RM) test_suite/*.o
	@if [ -d $(BUILD_PATH) ]; 		\
	then 							\
		rm -R $(BUILD_PATH); 		\
		echo "rm -R $(BUILD_PATH)" ;\
	fi
