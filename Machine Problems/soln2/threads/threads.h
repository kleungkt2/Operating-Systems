#ifndef THREADS_H_
#define THREADS_H_

#include <setjmp.h>

#define STACK_SIZE 8192 /* size of stack for one thread */

struct thread {
	void (*function)(void *); /* thread function */
	void *argument; /* argument of thread function */
	char *stack; /* pointer to the begining of the stack of the thread */
	char *stack_end; /* pointer to the end of the stack of the thread */
	jmp_buf env; /* context of thread */
	int init; /* is base and stackpointer initialized? */
	int tid; /* threadID */
	struct thread *next; /* next thread in ring */
};


struct thread *thread_create(void (*f)(void *), void *arg);
void thread_add_runqueue(struct thread *t);
void thread_yield(void);
void dispatch(void);
void schedule(void);
void thread_exit(void);
void thread_start_threading(void);

#endif // THREADS_H_
