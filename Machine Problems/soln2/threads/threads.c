/*
 * Master Solution Assignmnet 10
 */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "threads.h"

static struct thread *current_thread = NULL;
static struct thread *previous_thread = NULL;
static int next_tid = 0;

/* Context used to return after all thread finished. */
static jmp_buf main_env;


struct thread *thread_create(void (*f)(void *), void *arg) {
	struct thread *t;

	/* allocate new thread structure */
	if ((t = (struct thread *) malloc(sizeof(struct thread))) == NULL) {
		perror("Failed to allocate thread structure.");
	}

	/* allocate stack */
	if ((t->stack = malloc(STACK_SIZE)) == NULL) {
		perror("Failed to allocate stack.");
	}
	/* because the stack grows down, we need the reference to the end of the stack */
	t->stack_end = t->stack + STACK_SIZE;
	
	t->function = f;
	t->argument = arg;
	t->init = 0;
	t->tid = next_tid++;
	
	printf("DEBUG: Thread %d created.\n", t->tid);

	return t;
}

void thread_add_runqueue(struct thread *t) {
	if (current_thread == NULL) { /* first thread */
		current_thread = t;
		current_thread->next = t;
	} else {
		t->next = current_thread->next;
		current_thread->next = t;
	}
	printf("DEBUG: Added thread %d to runqueue.\n", t->tid);
}

void thread_yield(void) {
	schedule();
	dispatch();
}

void dispatch(void) {
	int ret;

	/* save old state - not needed if previous thread exited */
	if (previous_thread != NULL) {
		ret = setjmp(previous_thread->env);
		if (ret) { /* return form longjmp */
			return;
		}
	}
	
	/* run next thread */
	if (current_thread->init) { /* resume previously run thread */
		longjmp(current_thread->env, 1);
	} else { /* start thread */
        /* 64 bit assembler */
		__asm__ __volatile__("mov %%rax,%%rsp"
									:
									:"a"(current_thread->stack_end));
		__asm__ __volatile__("mov %%rax,%%rbp"
									:
									:"a"(current_thread->stack_end));
        /* 32 bit assembler */
		/*__asm__ __volatile__("mov %%eax,%%esp"
									:
									:"a"(current_thread->stack_end));
		__asm__ __volatile__("mov %%eax,%%ebp"
									:
									:"a"(current_thread->stack_end));
*/
		current_thread->init = 1;
		(*(current_thread->function))(current_thread->argument);
		thread_exit();
	}
}

void schedule(void) {
	previous_thread = current_thread; /* used to simplify dispatcher */
	current_thread = current_thread->next; /* RoundRobin Scheduler */
}

void thread_exit(void) {

	int only_one_left = 0; /* flag for only one thread left */
	if (current_thread->next == current_thread) {
		only_one_left = 1;
		printf("DEBUG: All threads finished their work.\n");
	}

	/* update ring */
	struct thread *previous = current_thread;
	while (previous->next != current_thread) {
		previous = previous->next;
	}
	previous->next = current_thread->next;

	/* free structures */
	printf("DEBUG: Remove thread %d from runqueue.\n", current_thread->tid);
	free(current_thread->stack);
	free(current_thread);

	if (!only_one_left) {
		current_thread = previous->next;
		previous_thread = NULL;
		dispatch();
	}

	longjmp(main_env, 1); /* no more threads left */
}

void thread_start_threading(void) {

	if (current_thread == NULL) {
		printf("DEBUG: There are no threads in the runqueue, exit.\n");
		return;
	}

	if (setjmp(main_env)) {
		return; /* no more threads left */
	} else {
		dispatch();
	}
}
