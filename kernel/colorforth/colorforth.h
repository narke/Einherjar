/*
 * Copyright (c) 2016, 2017, 2020 Konstantin Tcholokachvili
 * All rights reserved.
 * Use of this source code is governed by a MIT license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <io/console.h>

#define FORTH_DICTIONARY 0
#define MACRO_DICTIONARY 1

typedef int32_t cell_t;

typedef struct colorforth_word
{
	cell_t                 name;
	void                  *code_address;
} word_t;

struct editor_args
{
	struct console *cons;
	uint32_t initrd_start;
	uint32_t initrd_end;
};

void editor(void *args);
cell_t pack(const char *word_name);
char *unpack(cell_t word);
void run_block(const cell_t nb_block);
void dot_s(void);
void dispatch_word(cell_t word);
word_t lookup_word(cell_t name, const bool_t force_dictionary);
void colorforth_initialize(void);
void erase_stack(void);
