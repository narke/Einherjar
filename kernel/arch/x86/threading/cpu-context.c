#include <lib/libc.h>
#include <arch/x86/mmu/segment.h>

#include "cpu-context.h"

struct cpu_state 
{
	uint16_t gs;
	uint16_t fs;
	uint16_t es;
	uint16_t ds;
	uint16_t ss; 
	uint16_t padding; // Unused
	uint32_t eax;
	uint32_t ebx;
	uint32_t ecx;
	uint32_t edx;
	uint32_t esi;
	uint32_t edi;
	uint32_t ebp;

	/* The followig declaratins must be kept untouched since this order is
		required by the iret instruction. */
	uint32_t error_code;
	uint32_t eip;
	uint32_t cs;
	uint32_t eflags;
}__attribute__((packed));


/* Describes an interrupted kernel thread's context */
struct cpu_kstate //thread_cpu_context
{
	struct cpu_state regs;
}__attribute__((packed));


static void core_routine(cpu_kstate_function_arg1_t *start_func,
				uint32_t start_arg)
	     __attribute__((noreturn));


static void core_routine(cpu_kstate_function_arg1_t *start_func,
				uint32_t start_arg)
{
	start_func(start_arg);
	
	for(;;);
}


void cpu_kstate_init(struct cpu_state **ctxt,
		cpu_kstate_function_arg1_t *start_func,
		uint32_t start_arg,
		uint32_t stack_base,
		uint32_t stack_size)
{
	/* We are initializing a Kernel thread's context */
	struct cpu_kstate *kctxt;

	uint32_t tmp_vaddr = stack_base + stack_size;
	uint32_t *stack = (uint32_t *)tmp_vaddr;

	/* Simulate a call to the core_routine() function: prepare its arguments */
	*(--stack) = start_arg;
	*(--stack) = (uint32_t)start_func;
	*(--stack) = 0; /* Return address of core_routine => force page fault */

	/*
	 * Setup the initial context structure, so that the CPU will execute
	 * the function core_routine() once this new context has beencrestored on CPU
	 */

	/* Compute the base address of the structure, which must be located
	 * below the previous elements */
	tmp_vaddr = ((uint32_t)stack) - sizeof(struct cpu_kstate);
	kctxt = (struct cpu_kstate *)tmp_vaddr;
	
	/* Initialize the CPU context structure */
	memset(kctxt, 0x0, sizeof(struct cpu_kstate));
	
	/* Tell the CPU context structure that the first instruction
	 * to execute will be that of the core_routine() function */
	kctxt->regs.eip = (uint32_t)core_routine;

	/* Setup the segment registers */
	kctxt->regs.cs =
		X86_BUILD_SEGMENT_REGISTER_VALUE(KERNEL_CODE_SEGMENT); /* Code */
	kctxt->regs.ds =
		X86_BUILD_SEGMENT_REGISTER_VALUE(KERNEL_DATA_SEGMENT); /* Data */
	kctxt->regs.es =
		X86_BUILD_SEGMENT_REGISTER_VALUE(KERNEL_DATA_SEGMENT); /* Data */
	kctxt->regs.ss =
		X86_BUILD_SEGMENT_REGISTER_VALUE(KERNEL_DATA_SEGMENT); /* Stack */

	/* fs and gs unused for the moment. */
			         
	/* The newly created context is initially interruptible */
	kctxt->regs.eflags = (1 << 9); /* set IF bit */
					         
	/* Finally, update the generic kernel/user thread context */
	*ctxt = (struct cpu_state *)kctxt;
}
