Roentgenium
===========


What is it?
-----------

OS kernel in Asm/C/Forth.


Testing
-------

Step 1: Get sources

	$ git clone https://github.com/narke/Roentgenium.git

Step 2: Compile

	$ make all

Step 3: Generate an ISO image

	$ make cdrom

Step 4: Run with qemu (by using 4Mb of RAM and the generated ISO image)

	$ qemu -m 4 -cdrom roentgenium.iso
