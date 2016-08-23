#pragma once

#include <io/console.h>

typedef int32_t cell_t;

void editor(struct console *cons, uint32_t initrd_start, uint32_t initrd_end);
