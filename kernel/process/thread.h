#ifndef _THREAD_H_
#define _THREAD_H_

#include <arch/all/queue.h>
#include <arch/all/types.h>
#include <arch/x86-all/process/cpu_context.h>
#include <memory_manager/physical_memory.h>


/*
 * The size of the stack of a kernel thread
 */
#define THREAD_KERNEL_STACK_SIZE (1*X86_PAGE_SIZE)

#define THREAD_MAX_NAMELEN 32

/* Forward declaration */
struct thread;


/*
 * The possible states of a valid thread
 */
typedef enum 
{
	THREAD_CREATED,	/**< Thread created, not fully initialized */
	THREAD_READY,	/**< Thread fully initialized or waiting for CPU after 
					  having been blocked or preempted */
	THREAD_RUNNING,	/**< Thread currently running on CPU */
	THREAD_BLOCKED,	/**< Thread waiting for I/O (+ in at LEAST one kwaitq) 
					  and/or sleeping (+ in NO kwaitq) */
	THREAD_ZOMBIE,	/**< Thread terminated execution, waiting to be deleted
					 by kernel */
} thread_state;


/*
 * Definition of the function executed by a kernel thread
 */
typedef void (*kernel_thread_start_routine_t)(void *arg);
                         

struct thread
{
	/* Kernel stack parameters */
	char kernel_stack[THREAD_KERNEL_STACK_SIZE];
	uint32_t kernel_stack_base_address;
	uint32_t kernel_stack_size;

	char name[THREAD_MAX_NAMELEN];
	
	thread_state state;

	struct cpu_state *cpu_state;

	//TAILQ_HEAD(, waitqueue_entry) kwaitq_list;

	TAILQ_ENTRY(thread) next;
}__attribute__ ((packed, aligned (8)));


/*
 * Initialize the subsystem responsible for thread management
 * 
 * Initialize the primary kernel thread so that it can be handled
 * the same way as an ordinary thread created by thread_create().
 */
uint32_t threading_setup(uint32_t init_thread_stack_base_address,
		uint32_t init_thread_stack_size);

struct thread *thread_create(const char *name,
		kernel_thread_start_routine_t start_func,
		void *start_arg);

inline void thread_set_current(struct thread *current_thread);

struct thread *thread_get_current();

#endif // _THREAD_H_

