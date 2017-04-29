#include <lib/libc.h>
#include <lib/status.h>
#include <arch/x86/interrupts/irq.h>

#include "thread.h"
#include "scheduler.h"

TAILQ_HEAD(, thread) kernel_threads;

static volatile struct thread *g_current_thread = NULL;

static void idle_thread()
{
    while (1)
    {
        asm("hlt\n");
    }
}

inline void thread_set_current(struct thread *current_thread)
{
	assert(current_thread->state == THREAD_READY);

	g_current_thread        = current_thread;
	g_current_thread->state = THREAD_RUNNING;
}

struct thread *thread_get_current(void)
{
	assert(g_current_thread->state == THREAD_RUNNING);
	return (struct thread *)g_current_thread;
}

void threading_setup(void)
{
	TAILQ_INIT(&kernel_threads);

	// Declare the idle thread
	struct thread *idle = thread_create("idle", idle_thread, NULL);
	assert(idle != NULL);

	idle->state = THREAD_READY;
	thread_set_current(idle);
}

struct thread *thread_create(const char *name,
		kernel_thread_start_routine_t start_func,
		void *start_arg)
{
	uint32_t flags;
	struct thread *new_thread;

	if (!start_func)
		return NULL;

	/* Allocate a new thread structure for the current running thread */
	new_thread = malloc(sizeof(struct thread));

	if (!new_thread)
		return NULL;

	/* Initialize the thread attributes */
	strzcpy(new_thread->name, ((name)?name:"[NONAME]"), THREAD_MAX_NAMELEN);
	new_thread->state = THREAD_CREATED;

	/* Allocate the stack for the new thread */
	new_thread->stack_base_address	= (uint32_t)malloc(THREAD_KERNEL_STACK_SIZE);
	new_thread->stack_size		= THREAD_KERNEL_STACK_SIZE;

	if (!new_thread->stack_base_address)
		goto undo_creation;

	/* Initialize the CPU context of the new thread */
	cpu_kstate_init(&new_thread->cpu_state,
			(cpu_kstate_function_arg1_t *)start_func,
			(uint32_t)start_arg,
			new_thread->stack_base_address,
			new_thread->stack_size);

	/* Add the thread in the global list */
	X86_IRQs_DISABLE(flags);
	TAILQ_INSERT_TAIL(&kernel_threads, new_thread, next);
	X86_IRQs_ENABLE(flags);


	/* Mark the thread as ready */
	if (scheduler_set_ready(new_thread) != KERNEL_OK)
		goto undo_creation;

	return new_thread;


undo_creation:
	if (new_thread->stack_base_address)
		free((uint32_t *)new_thread->stack_base_address);

	free(new_thread);
	return NULL;
}
