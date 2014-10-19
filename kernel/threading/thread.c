#include <lib/libc.h>
#include <lib/status.h>
#include <arch/x86/interrupts/irq.h>

#include "thread.h"
#include "scheduler.h"

TAILQ_HEAD(, thread) kernel_threads;


static volatile struct thread *g_current_thread = NULL;


inline void thread_set_current(struct thread *current_thread)
{
	assert(current_thread->state == THREAD_READY);
	
	g_current_thread        = current_thread;
	g_current_thread->state = THREAD_RUNNING;
}

struct thread *thread_get_current()
{
	assert(g_current_thread->state == THREAD_RUNNING);
	return (struct thread *)g_current_thread;
}

ret_t threading_setup(uint32_t init_thread_stack_base_address,
		uint32_t init_thread_stack_size)
{
	struct thread *myself;
			          
	TAILQ_INIT(&kernel_threads);

	/* Allocate a new thread structure for the current running thread */
	myself = (struct thread *)malloc(sizeof(struct thread));

	if (!myself)
		return -KERNEL_NO_MEMORY;

	/* Initialize the thread attributes */
	strzcpy(myself->name, "[kinit]", THREAD_MAX_NAMELEN);
	myself->state                     = THREAD_CREATED;
	myself->kernel_stack_base_address = init_thread_stack_base_address;
	myself->kernel_stack_size         = init_thread_stack_size;
	
	/* Add the thread in the global list */        
	TAILQ_INSERT_TAIL(&kernel_threads, myself, next);
																						        
	/* Ok, now pretend that the running thread is ourselves */
	myself->state = THREAD_READY;
	thread_set_current(myself);
																										        
	return KERNEL_OK;
}



struct thread *thread_create(const char *name,
		kernel_thread_start_routine_t start_func,
		void *start_arg)
{
	__label__ undo_creation;
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
	new_thread->kernel_stack_base_address = (uint32_t)malloc(THREAD_KERNEL_STACK_SIZE);
	new_thread->kernel_stack_size         = THREAD_KERNEL_STACK_SIZE;
																									          
	if (!new_thread->kernel_stack_base_address)
		goto undo_creation;

	/* Initialize the CPU context of the new thread */
	cpu_kstate_init(&new_thread->cpu_state,
			(cpu_kstate_function_arg1_t *)start_func,
			(uint32_t)start_arg,
			new_thread->kernel_stack_base_address,
			new_thread->kernel_stack_size);

	/* Add the thread in the global list */
	X86_IRQs_DISABLE(flags);
	TAILQ_INSERT_TAIL(&kernel_threads, new_thread, next);
	X86_IRQs_ENABLE(flags);


	/* Mark the thread as ready */
	if (scheduler_set_ready(new_thread) != KERNEL_OK)
		goto undo_creation;
	
	return new_thread;


undo_creation:
	if (new_thread->kernel_stack_base_address)
		free((uint32_t *)new_thread->kernel_stack_base_address);

	free(new_thread);
	return NULL;
}
