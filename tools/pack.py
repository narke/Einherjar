#!/usr/bin/env python

# Copyright (c) Andrei Dragomir.
# All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import sys

code  = ' rtoeani' + 'smcylgfw' + 'dvpbhxuq' + '01234567' + \
        '89j-k.z/' + ';:!+@*,?'

asm_encodings = [
    '__',
    '_r',
    '_t',
    '_o',
    '_e',
    '_a',
    '_n',
    '_i',

    '_s',
    '_m',
    '_c',
    '_y',
    '_l',
    '_g',
    '_f',
    '_w',

    '_d',
    '_v',
    '_p',
    '_b',
    '_h',
    '_x',
    '_u',
    '_q',

    '_0',
    '_1',
    '_2',
    '_3',
    '_4',
    '_5',
    '_6',
    '_7',

    '_8',
    '_9',
    '_j',
    '_dash',
    '_k',
    '_dot',
    '_z',
    '_slash',

    '_semi',
    '_colon',
    '_store',
    '_plus',
    '_fetch',
    '_times',
    '_comma',
    '_question',
]

huffman_encodings = [
    0b0000, #
    0b0001, #r
    0b0010, #t
    0b0011, #o
    0b0100, #e
    0b0101, #a
    0b0110, #n
    0b0111, #i

    0b10000, #s
    0b10001, #m
    0b10010, #c
    0b10011, #y
    0b10100, #l
    0b10101, #g
    0b10110, #f
    0b10111, #w

    0b1100000, #d
    0b1100001, #v
    0b1100010, #p
    0b1100011, #b
    0b1100100, #h
    0b1100101, #x
    0b1100110, #u
    0b1100111, #q

    0b1101000, #0
    0b1101001, #1
    0b1101010, #2
    0b1101011, #3
    0b1101100, #4
    0b1101101, #5
    0b1101110, #6
    0b1101111, #7

    0b1110000, #8
    0b1110001, #9
    0b1110010, #j
    0b1110011, #-
    0b1110100, #k
    0b1110101, #.
    0b1110110, #z
    0b1110111, #/

    0b1111000, #;
    0b1111001, #:
    0b1111010, #!
    0b1111011, #+
    0b1111100, #@
    0b1111101, #*
    0b1111110, #,
    0b1111111, #?
]

highbit = 0x80000000L
mask    = 0xffffffffL

def packword_num(word):
    """pack a word into a 32-bit integer like colorForth editor does
    this routine ignores anything past 28 bits"""
    packed, bits = 0, 28
    for letter in word:
        lettercode = code.index(letter)
        length = 4 + (lettercode > 7) + (2 * (lettercode > 15))  # using True as 1
        lettercode += (8 * (length == 5)) + ((96 - 16) * (length == 7))  # True=1
        packed = (packed << length) + lettercode
        bits -= length
    packed <<= bits + 4
    return packed

def packword(word):
    """pack a word into a 32-bit integer like colorForth editor does
    this routine ignores anything past 28 bits"""
    packed, bits = 0, 28
    letter_codes = []
    lengths = []
    for i in range(0, len(word)):
        letter = word[i]
        #lettercode = huffman_encodings[code.index(letter)]
        lettercode = code.index(letter)
        length = 4 + (lettercode > 7) + (2 * (lettercode > 15))  # using True as 1
        lettercode += (8 * (length == 5)) + ((96 - 16) * (length == 7))  # True=1
        letter_codes.append(lettercode)
        lengths.append(length)
        packed = (packed << length) + lettercode
    s = sum(lengths)
    if s < 32:
        coded_word = "(" * (len(word) - 1) + asm_encodings[code.index(word[0])]
        i = 1
        while i < len(word):
            letter = word[i]
            coded_asm_letter = asm_encodings[code.index(letter)]
            displacement_for_this_letter = lengths[i]
            coded_word += "<<" + str(displacement_for_this_letter) + "|" + coded_asm_letter + ")"
            i = i+1
        coded_word += "<<" + str(32 - s)

if __name__ == "__main__":
    word = sys.argv[1]
    packword(word)
    packed = packword_num(word)
    print "0x%x" % packed
