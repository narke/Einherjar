#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os
import struct

INITRD_DIR_PATH = "../initrd"
INITRD_FILENAME = "initrd.img"

files = {}

def add_file(file_path):
	files[file_path] = os.stat(file_path).st_size


def visit_path(arg, directory, files):
    for filename in files:
        if os.path.isfile(directory + '/' + filename):
			add_file(directory + '/' +  filename)


def make_initrd(directory_path):
	if not os.path.isdir(directory_path):
		print("Error: Directory '%s' doesn't exits!" % directory_path)
		return 0
	
	# Gather infos about files
	os.path.walk(directory_path, visit_path, None)

	# Write files into ram disk image
	nb_files = len(files.keys())

	archive = open(INITRD_FILENAME, "wb")

	for file_path in files.keys():
		f = open(file_path, "rb")
		archive.write(f.read()) # Insert file's content
		f.close()

	archive.close()

	return nb_files


if __name__ == '__main__':
	if len(sys.argv) != 1:
		print("Usage: %s" % sys.argv[0])
		sys.exit()

	nb_files = make_initrd(INITRD_DIR_PATH)

	if nb_files > 0:
		print("Successfully generated a ram disk image: %s" % INITRD_FILENAME)
	else:
		print("Initrd generation failed: wrong path or there aren't files for initrd!")
