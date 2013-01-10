Roentgenium
===========

OS kernel in Asm/C/Forth.

How To Test
===========

[Step 1: Get versatile sources]

$ git clone https://github.com/narke/Roentgenium.git

[Step 2: Compile]

$ make all

[Step 3: Run with qemu (by using 4Mb of RAM and the generated ISO image)]

$ qemu -m 4 -cdrom roentgenium.iso
