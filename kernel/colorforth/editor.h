#ifndef _EDITOR_H_
#define _EDITOR_H_

#include <io/console.h>

typedef int32_t cell_t;

void editor(struct console *cons, uint32_t initrd_start);

#endif // _EDITOR_H_
