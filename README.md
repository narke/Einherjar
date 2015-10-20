Roentgenium
===========


What is it?
-----------

A prototype of a real-time operating system.
This will be a ColorForth computing environment, without separating
the operating system from the programming language and applications.


Screenshot
----------
![alt text](https://raw.githubusercontent.com/narke/Roentgenium/master/docs/screenshots/roentgenium.gif "Roentgenium")


Testing
-------

Step 1: Get sources

	$ git clone https://github.com/narke/Roentgenium.git

Step 2: Compile and generate an ISO image

	$ cd Roentgenium/kernel
	$ make all

Step 3: Run with qemu (by using 4Mb of RAM and the generated ISO image)

	$ qemu -m 4 -cdrom ../build/roentgenium.iso

Step 4: Clean your build if you want

	$ make clean
