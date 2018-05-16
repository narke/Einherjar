#!/usr/bin/env python2

# Copyright (c) Andrei Dragomir.
# All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import sys, os, struct, array, re, pprint
import pyparsing as p

code  = ' rtoeani' + 'smcylgfw' + 'dvpbhxuq' + '01234567' + '89j-k.z/' + ';:!+@*,?'

hexadecimal = '0123456789abcdef'

functions = [
    "extension", "execute", "executelong", "define",
    "compileword", "compilelong", "compileshort", "compilemacro",
    "executeshort", "text", "textcapitalized", "textallcaps",
    "variable", "compiler_feedback", "display_macro", "commented_number"
]

fivebit_functions = {
    "hex_executelong": functions.index('executelong'),
    "hex_executeshort": functions.index('compilelong'),
    "hex_compileshort": functions.index('compileshort'),
    "hex_compilelong": functions.index('executeshort'),
    "hex_commented_number": functions.index('commented_number')
}

accepted_functions = ' '.join(functions) + ' ' + ' '.join(fivebit_functions.keys())

highbit = 0x80000000L
mask = 0xffffffffL

def unpack(coded):
    bits = 32 - 4  # 28 bits used for compressed text
    coded &= ~0xf  # so zero low 4 bits
    text = ''
    while coded:
        nibble = coded >> 28
        coded = (coded << 4) & mask
        bits -= 4
        if nibble < 0x8:  # 4-bit coded character
            text += code[nibble]
        elif nibble < 0xc: # 5-bit code
            text += code[(((nibble ^ 0xc) << 1) | (coded & highbit > 0))]
            coded = (coded << 1) & mask
            bits -= 1
        else:  # 7-bit code
            text += code[(coded >> 29) + (8 * (nibble - 10))]
            coded = (coded << 3) & mask
            bits -= 3
    return text

def packword(word):
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
    if word != unpack(packed):
        raise Exception('packword: error: word "%s" packed as 0x%08x, "%s"\n' % (word, packed, unpack(packed)))
    else:
        return packed

class ParseTextToCFBlock:
    def __init__(self):
        self.out = array.array('I')
        self.out_per_block = array.array('I')
        self.block_number = 0
        self.in_variable = False

    def block_action(self, s, loc, toks):
        self.block_number = int(toks[1], 10)
        if len(self.out_per_block) < 256:
            for x in range(0, (256 - len(self.out_per_block))):
                self.out_per_block.append(0)
        else:
            print "FUCKED!!!"
        self.out.extend(self.out_per_block)
        self.out_per_block = array.array('I')

    def function_action(self, s, loc, toks):
        function_name = toks[0]
        function_param = toks[2]
        if function_name == 'variable':
            self.in_variable = True
        self.out_per_block.extend(self.word_to_long_array(function_name, function_param))

    def get_tags_for_function(self, f):
        """returns a 4 bit number"""
        start = 0
        if f in fivebit_functions.keys():
            start = 0b10000
            f = f[4:]
        return start | functions.index(f)

    def huffman_for_char(self, c):
        lettercode = code.index(c)
        hl = 4 + (lettercode > 7) + (2 * (lettercode > 15))  # using True as 1
        hc = lettercode + (8 * (hl == 5)) + ((96 - 16) * (hl == 7))  # True=1
        return (hl, hc)

    def word_to_long_array(self, f, param):
        """pack a word into list of integers"""
        packed = 0
        tmp = array.array('I')
        tag = self.get_tags_for_function(f)
        if f in ['hex_compilelong', 'hex_executelong', 'compilelong', 'executelong']:
            # two words
            tmp.append(tag)
            if tag & 0b10000: # is hex ?
                actual_number = int(param, 16) & mask
            else:
                actual_number = int(param, 10) & mask
            tmp.append(actual_number)
        elif f in ['compileshort', 'executeshort', 'hex_compileshort', 'hex_executeshort', 'commented_number', 'hex_commented_number']:
            if tag & 0b10000: # is hex ?
                actual_number = int(param, 16) & mask
            else:
                actual_number = int(param, 10) & mask
            packed = ((actual_number << 5) & mask) + tag
            tmp.append(packed)
        elif f == 'compileword' and self.in_variable == True:
            self.in_variable = False
            actual_number = int(param, 10) & mask
            tmp.append(actual_number)
        else:
            extension_mode = False
            bits = 28
            idx = 0
            extension_mode = False
            while idx < len(param):
                while 1:
                    if bits <= 0 or idx >= len(param):
                        break
                    hl, hc = self.huffman_for_char(param[idx])
                    if bits >= hl:
                        packed = ((packed << hl) & mask) | hc
                        bits -= hl
                        idx += 1
                    else:
                        # try to shrink the character
                        more_shrink = True
                        shrinked = hc
                        copy_hc = hc
                        while more_shrink:
                            if bits >= hl:
                                packed = ((packed << hl) & mask) ^ shrinked
                                bits -= hl
                                idx += 1
                                break
                            else:
                                if shrinked & 0x1 == 1:
                                    # goto full
                                    self.finish_pack(tmp, packed, bits, (0 if extension_mode else tag))
                                    bits = 28
                                    packed = 0
                                    extension_mode = True
                                    more_shrink = False
                                else:
                                    shrinked = (shrinked >> 1)
                                    hl -= 1
                if (bits != 28 and packed != 0):
                    self.finish_pack(tmp, packed, bits, (0 if extension_mode else tag))
                    extension_mode = True
                    bits = 28
                    packed = 0
        return tmp

    def finish_pack(self, tmp, packed, bits, tag):
        packed = (packed << (bits + 4)) & mask
        packed = packed | tag
        tmp.append(packed)

    def parse_input(self, s):
        function = (p.oneOf(accepted_functions) + "(" + p.CharsNotIn(")") + ")" + p.White()).setParseAction(self.function_action)
        block = ("{block " + p.Word(p.nums) + "}" + p.ZeroOrMore(function)).setParseAction(self.block_action)
        cf2text = p.OneOrMore(block)
        cf2text.parseString(s)

chars = [' ', 'r', 't', 'o', 'e', 'a', 'n', 'i',
    's', 'm', 'c', 'y', 'l', 'g', 'f', 'w',
    'd', 'v', 'p', 'b', 'h', 'x', 'u', 'q',
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'j', '-', 'k', '.', 'z', '/',
    ';', ':', '!', '+', '@', '*', ',', '?']

hex_chars = ['0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'a', 'b', 'c', 'd', 'e', 'f']


class ParseCFBlockToText:
    def __init__(self):
        self.out = ''

    def print_hex(self, i):
        n, f = (8, 0)
        if i == 0:
            self.out += '0'
            return
        while n > 0:
            n -= 1
            if not (i & 0xf0000000):
                if f:
                    self.out += '0'
                else:
                    f = 1;
                    self.out += hex_chars[i >> 28]
            i = (i << 4) & mask

def print_dec(self, i):
    j, k, f = (0, 0, 0)
    if i == 0:
        self.out += '0'
        return
    if i < 0:
        self.out += '-'
        i = -i;
    j = 1000000000
    while j != 0:
        k = i / j
        if k == 0:
            if f:
                self.out += '0'
        else:
            i -= j * k
            f = 1
            self.out += hex_chars[k]
            j /= 10

def print_hex_or_dec(self, n, word):
    if (word & 0x10):
        self.print_hex(n)
    else:
        self.print_dec(n)

def print_text(self, word):
    while word:
        self.has_output = 1;
        if not (word & 0x80000000):
            self.out += chars[word >> 28]
            word = (word << 4) & mask
        elif (word & 0xc0000000) == 0x80000000:
            self.out += chars[8 + ((word >> 27) & 7)]
            word = (word << 5) & mask
        else:
            self.out += (chars[((word >> 28) - 10) * 8 + ((word >> 25) & 7)]);
            word = (word << 7) & mask

def print_tags(self, end_word, typebits, hexbit):
    if end_word:
        self.out += ") "
    if typebits == 3 and end_word:
        self.out += "\n"
    if hexbit > 0:
        self.out += "hex_%s(" % functions[typebits]
    else:
        self.out +=  "%s(" % functions[typebits]

def allzero(self, array):
    return not filter(long.__nonzero__, map(long, array))

def parse_input(self, contents):
    self.contents = contents
    end_word = 0
    for block in range(len(self.contents) / 1024):
        chunk = self.contents[block * 1024:(block * 1024) + 1024]
        block_data = struct.unpack('<256l', chunk)
        self.out += '{block %d}\n' % block
        if not self.allzero(block_data):
            pos = 0
            while pos < 256:
                word = block_data[pos]
                tmp_tag = word & 0xf
                if tmp_tag == 0:
                    self.print_text(word & 0xfffffff0)
                elif tmp_tag in [2, 5]:
                    self.print_tags(end_word, word & 0xf, word & 0x10);
                    if (pos == 255):
                        break
                    else:
                        pos += 1
                        another_word = block_data[pos]
                        self.print_hex_or_dec(another_word, word)
                elif tmp_tag in [6, 8]:
                    self.print_tags(end_word, word & 0xf, word & 0x10)
                    self.print_hex_or_dec(word >> 5, word)
                elif tmp_tag == 0xf:
                    self.print_tags(end_word, word & 0xf, word & 0x10)
                    self.print_hex_or_dec(word >> 5, word)
                elif tmp_tag == 0xc:
                    self.print_tags(end_word, word & 0xf, 0)
                    self.print_text(word & 0xfffffff0)
                    if pos == 255:
                        break
                    else:
                        pos += 1
                        word = block_data[pos]
                        self.print_tags(1, 4, 0)
                        self.print_dec(word)
                else:
                    self.print_tags(end_word, word & 0xf, 0);
                    self.print_text(word & 0xfffffff0);
                end_word = 1
                pos += 1
            if end_word and self.has_output:
                self.out += ")\n"
            end_word = 0
            has_output = 0

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print "at least one argument"
        sys.exit(1)
    if sys.argv[1] == "tocf":
        parser = ParseTextToCFBlock()
        input_f = open(sys.argv[2], "rb")
        input_contents = input_f.read()
        input_f.close()
        parser.parse_input(input_contents)
        f = open(sys.argv[3], "wb+")
        parser.out.tofile(f);
        f.close()
    elif sys.argv[1] == "totext":
        parser = ParseCFBlockToText()
        input_f = open(sys.argv[2], "rb")
        input_contents = input_f.read()
        input_f.close()
        parser.parse_input(input_contents)
        f = open(sys.argv[3], "wb+")
        f.write(parser.out)
        f.close()
