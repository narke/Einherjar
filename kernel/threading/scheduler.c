#include <lib/libc.h>

#include "scheduler.h"


TAILQ_HEAD(, thread) ready_threads_queue;

void scheduler_setup(void)
{
	TAILQ_INIT(&ready_threads_queue);
}


/*
 * Helper function to add a thread in a ready queue AND to change
 * the state of the given thread to "READY".
 *
 * @param insert_at_tail TRUE to tell to add the thread at
 * the end of the ready list. Otherwise it is added at the head of it.
 */

static void add_in_ready_queue(struct thread *thr,
									bool_t insert_at_tail)
{
	assert((THREAD_CREATED == thr->state)
		|| (THREAD_RUNNING == thr->state) /* Yield */
		|| (THREAD_BLOCKED == thr->state) );

	/* Ok, thread is now really ready to be (re)started */
	thr->state = THREAD_READY;

	/* Add the thread to the threads queue */
	if (insert_at_tail)
		TAILQ_INSERT_TAIL(&ready_threads_queue, thr, next);
	else
		TAILQ_INSERT_HEAD(&ready_threads_queue, thr, next);
}

uint32_t scheduler_set_ready(struct thread *thr)
{
	/* Don't do anything for already ready threads */
	if (THREAD_READY == thr->state)
		return KERNEL_OK;

	/* Real-time thread: schedule it for the present turn */
	add_in_ready_queue(thr, TRUE);

	return KERNEL_OK;
}

void schedule(void)
{
	struct thread *current_thread;
	struct thread *next_thread;

	current_thread = thread_get_current();

	if (current_thread->state != THREAD_BLOCKED)
		add_in_ready_queue(current_thread, TRUE);

	assert(TAILQ_EMPTY(&ready_threads_queue) == FALSE);

	next_thread = TAILQ_FIRST(&ready_threads_queue);
	TAILQ_REMOVE(&ready_threads_queue, next_thread, next);
	TAILQ_INSERT_TAIL(&ready_threads_queue, next_thread, next);

	thread_set_current(next_thread);

	// Avoid context switch if the context does not change
	if (current_thread != next_thread)
	{
		cpu_context_switch(&current_thread->cpu_state, next_thread->cpu_state);

		assert(current_thread == thread_get_current());
		assert(current_thread->state == THREAD_RUNNING);
	}
}
