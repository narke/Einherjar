/*
 * Copyright (c) 2016 Konstantin Tcholokachvili
 * All rights reserved.
 * Use of this source code is governed by a MIT license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <io/console.h>

typedef int32_t cell_t;

void editor(struct console *cons, uint32_t initrd_start, uint32_t initrd_end);
cell_t pack(const char *word_name);
char *unpack(cell_t word);
void run_block(const cell_t nb_block);
void dot_s(void);
void colorforth_initialize(void);
void colorforth_finalize(void);
