/*
 * Copyright (c) 2016, 2017 Konstantin Tcholokachvili
 * All rights reserved.
 * Use of this source code is governed by a MIT license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <io/console.h>

#define FORTH_DICTIONARY TRUE
#define MACRO_DICTIONARY FALSE

typedef int32_t cell_t;
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
void do_word(cell_t word);
struct word_entry *lookup_word(cell_t name, const bool_t force_dictionary);
void colorforth_initialize(void);
void colorforth_finalize(void);
