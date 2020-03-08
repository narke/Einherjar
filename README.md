Einherjar
===========

[![Coverity Status](https://scan.coverity.com/projects/10390/badge.svg)](https://scan.coverity.com/projects/10390)


What is it?
-----------

A prototype of a real-time operating system.  
This will be a ![colorForth](https://raw.githubusercontent.com/narke/Einherjar/master/docs/colorforth.png "colorForth")
computing environment, without separating
the operating system from the programming language and applications.



Screenshot
----------
![Screenshot](https://raw.githubusercontent.com/narke/Einherjar/master/docs/screenshots/roentgenium.gif "Einherjar")


Testing
-------

Step 1: Get sources

	$ git clone https://github.com/narke/Einherjar.git

Step 2: Compile and generate an ISO image

	$ cd Einherjar/kernel
	$ make all

Step 3: Run with qemu (by using 4Mb of RAM and the generated ISO image)

	$ qemu-system-i386 -m 4 -cdrom ../build/roentgenium.iso

Step 4: Clean your build if you want

	$ make clean
