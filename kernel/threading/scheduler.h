#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include <lib/types.h>

#include "thread.h"

void scheduler_setup(void);

uint32_t scheduler_set_ready(struct thread * thr);

void schedule(void);

#endif // _SCHEDULER_H_
